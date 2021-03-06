
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "TableApplications.h"
#include "TableAttributes.h"
#include "TableMusicGenres.h"
#include <algorithm>
#include <hash_map>
#include <math.h>
#include <shlwapi.h>


extern WCHAR ItemColorNames[LFItemColorCount-1][256];


#pragma comment(lib, "shlwapi.lib")


// Internal functions
//

#define ROUNDOFF 0.00000001

__forceinline DOUBLE GetMinutes(DOUBLE c)
{
	c = fabs(c)+ROUNDOFF;

	return (c-(DOUBLE)(INT)c)*60.0;
}

__forceinline DOUBLE GetSeconds(DOUBLE c)
{
	c = fabs(c)*60.0+ROUNDOFF;

	return (c-(DOUBLE)(INT)c)*60.0;
}

BOOL IsNullValue(UINT Type, LPCVOID pValue)
{
	if (!pValue)
		return TRUE;

	assert(Type<LFTypeCount);

	switch (Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (*(LPCWSTR)pValue)==L'\0';

	case LFTypeAnsiString:
	case LFTypeIATACode:
		return (*(LPCSTR)pValue)=='\0';

	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeBitrate:
	case LFTypeDuration:
	case LFTypeGenre:
	case LFTypeYear:
	case LFTypeFramerate:
		return (*(UINT*)pValue)==0;

	case LFTypeRating:
	case LFTypeColor:
	case LFTypeApplication:
		return (*(BYTE*)pValue)==0;

	case LFTypeSize:
	case LFTypeTime:
		return (*(INT64*)pValue)==0;

	case LFTypeFraction:
		return (((LFFraction*)pValue)->Num==0) || (((LFFraction*)pValue)->Denum==0);

	case LFTypeDouble:
	case LFTypeMegapixel:
		return (*(DOUBLE*)pValue)==-1;

	case LFTypeGeoCoordinates:
		return (((LFGeoCoordinates*)pValue)->Latitude==0) && (((LFGeoCoordinates*)pValue)->Longitude==0);
	}

	return FALSE;
}

INT CompareValues(UINT Type, LPCVOID pData1, LPCVOID pData2, BOOL CaseSensitive)
{
	assert(Type<LFTypeCount);
	assert(pData1);
	assert(pData2);

	UINT UInt1;
	UINT UInt2;
	DOUBLE Double1;
	DOUBLE Double2;

	switch (Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return CaseSensitive ? wcscmp((LPCWSTR)pData1, (LPCWSTR)pData2) : _wcsicmp((LPCWSTR)pData1, (LPCWSTR)pData2);

	case LFTypeAnsiString:
	case LFTypeIATACode:
		return CaseSensitive ? strcmp((LPCSTR)pData1, (LPCSTR)pData2) : _stricmp((LPCSTR)pData1, (LPCSTR)pData2);

	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeGenre:
	case LFTypeYear:
	case LFTypeFramerate:
		return *(UINT*)pData1==*(UINT*)pData2 ? 0 : *(UINT*)pData1<*(UINT*)pData2 ? -1 : 1;

	case LFTypeRating:
	case LFTypeColor:
		return (INT)(*(BYTE*)pData1)-(INT)(*(BYTE*)pData2);

	case LFTypeSize:
		return *(INT64*)pData1==*(INT64*)pData2 ? 0 : *(INT64*)pData1<*(INT64*)pData2 ? -1 : 1;

	case LFTypeDouble:
		Double1 = (DOUBLE)((INT)(*(DOUBLE*)pData1*100.0))/100.0;
		Double2 = (DOUBLE)((INT)(*(DOUBLE*)pData2*100.0))/100.0;

		return Double1==Double2 ? 0 : Double1<Double2 ? -1 : 1;

	case LFTypeGeoCoordinates:
		if (((LFGeoCoordinates*)pData1)->Latitude==((LFGeoCoordinates*)pData2)->Latitude)
		{
			return ((LFGeoCoordinates*)pData1)->Longitude==((LFGeoCoordinates*)pData2)->Longitude ? 0 : ((LFGeoCoordinates*)pData1)->Longitude<((LFGeoCoordinates*)pData2)->Longitude ? -1 : 1;
		}
		else
		{
			return ((LFGeoCoordinates*)pData1)->Latitude<((LFGeoCoordinates*)pData2)->Latitude ? -1 : 1;
		}

	case LFTypeTime:
		return CompareFileTime((LPFILETIME)pData1, (LPFILETIME)pData2);

	case LFTypeBitrate:
	case LFTypeDuration:
		UInt1 = *(UINT*)pData1/1000;
		UInt2 = *(UINT*)pData2/1000;

		return UInt1==UInt2 ? 0 : UInt1<UInt2 ? -1 : 1;

	case LFTypeMegapixel:
		Double1 = (DOUBLE)((INT)(*(DOUBLE*)pData1*10.0))/10.0;
		Double2 = (DOUBLE)((INT)(*(DOUBLE*)pData2*10.0))/10.0;

		return (Double1==Double2) ? 0 : Double1<Double2 ? -1 : 1;

	case LFTypeApplication:
		UInt1 = *((BYTE*)pData1);
		UInt2 = *((BYTE*)pData2);

		if ((UInt1>=LFApplicationCount) && (UInt2>=LFApplicationCount))
			return 0;

		if (UInt1>=LFApplicationCount)
			return -1;

		if (UInt2>=LFApplicationCount)
			return 1;

		return wcscmp(ApplicationRegistry[UInt1].Name, ApplicationRegistry[UInt2].Name);
	}

	return 0;
}

void ToString(LPCVOID pValue, UINT Type, LPWSTR pStr, SIZE_T cCount)
{
	assert(Type<LFTypeCount);
	assert(pStr);

	if (pValue)
		switch (Type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(pStr, cCount, (LPCWSTR)pValue);
			return;

		case LFTypeAnsiString:
		case LFTypeIATACode:
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pValue, -1, pStr, (INT)cCount);
			return;

		case LFTypeFourCC:
			LFFourCCToString(*((UINT*)pValue), pStr, cCount);
			return;

		case LFTypeRating:
			assert(*((BYTE*)pValue)<=LFMaxRating);

			if (*((BYTE*)pValue)!=1)
			{
				swprintf_s(pStr, cCount, L"%u", (UINT)(*((BYTE*)pValue)/2));
			}
			else
			{
				pStr[0] = L'\0';
			}

			if (*((BYTE*)pValue)%2)
				wcscat_s(pStr, cCount, L"?");

			return;

		case LFTypeColor:
			wcscpy_s(pStr, cCount, *((BYTE*)pValue) ? ItemColorNames[*((BYTE*)pValue)-1] : L"");
			return;

		case LFTypeUINT:
		case LFTypeYear:
			LFUINTToString(*((UINT*)pValue), pStr, cCount);
			return;

		case LFTypeSize:
			LFSizeToString(*((INT64*)pValue), pStr, cCount);
			return;

		case LFTypeFraction:
			LFFractionToString(*((LFFraction*)pValue), pStr, cCount);
			return;

		case LFTypeDouble:
			LFDoubleToString(*((DOUBLE*)pValue), pStr, cCount);
			return;

		case LFTypeGeoCoordinates:
			LFGeoCoordinatesToString(*((LFGeoCoordinates*)pValue), pStr, cCount, FALSE);
			return;

		case LFTypeTime:
			LFTimeToString(*((LPFILETIME)pValue), pStr, cCount);
			return;

		case LFTypeBitrate:
			LFBitrateToString(*((UINT*)pValue), pStr, cCount);
			return;

		case LFTypeDuration:
			LFDurationToString(*((UINT*)pValue), pStr, cCount);
			return;

		case LFTypeMegapixel:
			LFMegapixelToString(*((DOUBLE*)pValue), pStr, cCount);
			return;

		case LFTypeGenre:
			wcscpy_s(pStr, cCount, GetGenreName(*((UINT*)pValue)));
			return;

		case LFTypeApplication:
			wcscpy_s(pStr, cCount, *((BYTE*)pValue)<LFApplicationCount ? ApplicationRegistry[*((BYTE*)pValue)].Name : L"?");
			return;

		case LFTypeFramerate:
			LFFramerateToString(*((UINT*)pValue), pStr, cCount);
			return;
		}

	*pStr = L'\0';
}

BOOL GetNextHashtag(LPCWSTR* ppUnicodeArray, LPWSTR pHashtag, SIZE_T cCount)
{
	LPCWSTR Start = NULL;
	BOOL InQuotation = FALSE;

	while (**ppUnicodeArray!=L'\0')
	{
		switch (**ppUnicodeArray)
		{
		case L' ':
		case L'.':
		case L',':
		case L':':
		case L';':
		case L'?':
		case L'!':
		case L'|':
			if ((Start) && (!InQuotation))
			{
				wcsncpy_s(pHashtag, cCount, Start, ((*ppUnicodeArray)++)-Start);

				return TRUE;
			}

			break;

		case L'"':
			if (InQuotation)
			{
				if (Start)
				{
					wcsncpy_s(pHashtag, cCount, Start, ((*ppUnicodeArray)++)-Start);

					return TRUE;
				}

				InQuotation = FALSE;
			}
			else
			{
				InQuotation = !Start;
			}

			break;

		default:
			if (!Start)
				Start = *ppUnicodeArray;
		}

		(*ppUnicodeArray)++;
	}

	if (Start)
	{
		wcscpy_s(pHashtag, cCount, Start);

		return TRUE;
	}

	return FALSE;
}

LFCORE_API BOOL LFContainsHashtag(LPCWSTR pUnicodeArray, LPCWSTR pHashtag)
{
	assert(pUnicodeArray);
	assert(pHashtag);

	WCHAR Hashtag[256];
	while (GetNextHashtag(&pUnicodeArray, Hashtag, 256))
		if (_wcsicmp(Hashtag, pHashtag)==0)
			return TRUE;

	return FALSE;
}


// ToString
//

LFCORE_API void LFFourCCToString(const UINT FourCC, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (cCount<5)
	{
		*pStr = L'\0';
	}
	else
	{
		pStr[0] = FourCC & 0xFF;
		pStr[1] = (FourCC>>8) & 0xFF;
		pStr[2] = (FourCC>>16) & 0xFF;
		pStr[3] = FourCC>>24;
		pStr[4] = '\0';
	}
}

LFCORE_API void LFUINTToString(const UINT Uint, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (!Uint)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u", Uint);
	}
}

LFCORE_API void LFSizeToString(const INT64 Size, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	StrFormatByteSize(Size, pStr, (UINT)cCount);
}

LFCORE_API void LFFractionToString(const LFFraction& Fraction, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if ((Fraction.Num==0) || (Fraction.Denum==0))
	{
		pStr[0] = L'\0';
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u/%u", Fraction.Num, Fraction.Denum);
	}
}

LFCORE_API void LFDoubleToString(const DOUBLE Double, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	swprintf_s(pStr, cCount, L"%.2lf", Double);
}

LFCORE_API void LFGeoCoordinateToString(const DOUBLE Coordinate, LPWSTR pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero)
{
	assert(pStr);

	swprintf_s(pStr, cCount, FillZero ? L"%03u?%02u\'%02u\"%c" : L"%u?%u\'%u\"%c",
		(UINT)(fabs(Coordinate)+ROUNDOFF),
		(UINT)GetMinutes(Coordinate),
		(UINT)(GetSeconds(Coordinate)+0.5),
		Coordinate>0 ? IsLatitude ? L'S' : L'E' : IsLatitude ? L'N' : L'W');
}

LFCORE_API void LFGeoCoordinatesToString(const LFGeoCoordinates& Coordinates, LPWSTR pStr, SIZE_T cCount, BOOL FillZero)
{
	assert(pStr);

	if ((Coordinates.Latitude==0) && (Coordinates.Longitude==0))
	{
		*pStr = L'\0';
	}
	else
	{
		WCHAR tmpStr[32];
		LFGeoCoordinateToString(Coordinates.Longitude, tmpStr, 32, FALSE, FillZero);

		LFGeoCoordinateToString(Coordinates.Latitude, pStr, cCount, TRUE, FillZero);
		wcscat_s(pStr, cCount, L", ");
		wcscat_s(pStr, cCount, tmpStr);
	}
}

LFCORE_API void LFTimeToString(const FILETIME& Time, LPWSTR pStr, SIZE_T cCount, BOOL IncludeTime)
{
	assert(pStr);

	*pStr = L'\0';

	if (Time.dwHighDateTime || Time.dwLowDateTime)
	{
		SYSTEMTIME stUTC;
		SYSTEMTIME stLocal;
		FileTimeToSystemTime(&Time, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

		const INT Size = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal, NULL, pStr, (INT)cCount);

		if (IncludeTime && (stLocal.wHour || stLocal.wMinute || stLocal.wSecond))
		{
			pStr[Size-1] = L' ';

			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &stLocal, NULL, &pStr[Size], (INT)cCount-Size);
		}
	}
}

LFCORE_API void LFBitrateToString(const UINT Bitrate, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (!Bitrate)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u kBit/s", (Bitrate+500)/1000);
	}
}

LFCORE_API void LFDurationToString(UINT Duration, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if ((Duration=(Duration+500)/1000)==0)
	{
		*pStr = L'\0';
	}
	else
		if (Duration/3600)
		{
			swprintf_s(pStr, cCount, L"%02u:%02u:%02u", Duration/3600, (Duration/60)%60, Duration%60);
		}
		else
		{
			swprintf_s(pStr, cCount, L"%02u:%02u", Duration/60, Duration%60);
		}
}

LFCORE_API void LFMegapixelToString(const DOUBLE Resolution, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	swprintf_s(pStr, cCount, L"%.1lf Megapixel", Resolution);
}

LFCORE_API void LFFramerateToString(const UINT Framerate, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (!Framerate)
	{
		*pStr = L'\0';
	}
	else
		if ((Framerate % 1000)==0)
		{
			swprintf_s(pStr, cCount, L"%u fps", Framerate/1000);
		}
		else
		{
			swprintf_s(pStr, cCount, L"%.2lf fps", Framerate/1000.0);
		}
}

LFCORE_API void LFAttributeToString(const LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr, LPWSTR pStr, SIZE_T cCount)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);
	assert(AttrProperties[Attr].Type<LFTypeCount);

	ToString(pItemDescriptor->AttributeValues[Attr], AttrProperties[Attr].Type, pStr, cCount);
}


// LFVariantData
//

LFCORE_API void LFInitVariantData(LFVariantData& VData, ATTRIBUTE Attr)
{
	VData.Attr = Attr;
	VData.Type = (Attr<LFAttributeCount) ? AttrProperties[Attr].Type : LFTypeUnicodeString;

	LFClearVariantData(VData);
}

LFCORE_API void LFClearVariantData(LFVariantData& VData)
{
	assert(VData.Type<LFTypeCount);

	VData.IsNull = TRUE;

	switch (VData.Type)
	{
	case LFTypeDouble:
		VData.Double = 0;
		break;

	case LFTypeGeoCoordinates:
		VData.GeoCoordinates.Latitude = VData.GeoCoordinates.Longitude = 0;
		break;

	case LFTypeMegapixel:
		VData.Double = -1;
		break;

	default:
		VData.INT64 = 0;
	}
}

LFCORE_API BOOL LFIsNullVariantData(const LFVariantData& VData)
{
	return VData.IsNull ? TRUE : IsNullValue(VData.Type, &VData.Value);
}

LFCORE_API void LFVariantDataToString(const LFVariantData& VData, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (VData.IsNull)
	{
		*pStr = L'\0';
	}
	else
	{
		ToString(&VData.Value, VData.Type, pStr, cCount);
	}
}

LFCORE_API void LFVariantDataFromString(LFVariantData& VData, LPCWSTR pStr)
{
	LFClearVariantData(VData);

	if (pStr)
	{
		const SIZE_T Size = wcslen(pStr);

		WCHAR Buffer[256];
		WCHAR* pChar;

		INT LatDeg;
		INT LatMin;
		INT LatSec;
		WCHAR LatCh;
		INT LonDeg;
		INT LonMin;
		INT LonSec;
		WCHAR LonCh;

		UINT Date1;
		WCHAR DateCh1;
		UINT Date2;
		WCHAR DateCh2;
		UINT Date3;
		SYSTEMTIME stLocal;
		SYSTEMTIME stUTC;
		BOOL YearFirst;

		UINT Hour;
		UINT Min;
		UINT Sec;

		switch (VData.Type)
		{
		case LFTypeUnicodeString:
			wcsncpy_s(VData.UnicodeString, 256, pStr, _TRUNCATE);
			VData.IsNull = FALSE;

			break;

		case LFTypeUnicodeArray:
			wcsncpy_s(VData.UnicodeArray, 256, pStr, _TRUNCATE);
			LFSanitizeUnicodeArray(VData.UnicodeArray, 256);
			VData.IsNull = FALSE;

			break;

		case LFTypeAnsiString:
		case LFTypeIATACode:
			WideCharToMultiByte(CP_ACP, 0, pStr, -1, VData.AnsiString, 256, NULL, NULL);
			VData.IsNull = FALSE;

			if (VData.Type==LFTypeIATACode)
			{
				VData.IATACode[0] = (CHAR)toupper(VData.IATACode[0]);
				VData.IATACode[1] = (CHAR)toupper(VData.IATACode[1]);
				VData.IATACode[2] = (CHAR)toupper(VData.IATACode[2]);
			}

			break;

		case LFTypeRating:
			if (wcscmp(pStr, L"?")==0)
			{
				VData.Rating = 1;
				VData.IsNull = FALSE;

				return;
			}

			if ((pStr[0]>=L'0') && (pStr[0]<=L'5'))
				if (Size<=2)
				{
					VData.Rating = (BYTE)((pStr[0]-L'0')*2);

					if (Size==2)
						if (pStr[1]==L'?')
							VData.Rating++;

					VData.IsNull = FALSE;

					return;
				}

			if ((Size>=1) && (Size<=5))
			{
				BOOL Same = TRUE;
				for (UINT a=1; a<Size; a++)
					Same &= (pStr[a]==pStr[a-1]);

				if (Same)
				{
					VData.Rating = (BYTE)(Size*2);
					VData.IsNull = FALSE;

					return;
				}
			}

			break;

		case LFTypeUINT:
		case LFTypeYear:
			if (swscanf_s(pStr, L"%u", &VData.UINT32)==1)
				VData.IsNull = FALSE;

			break;

		case LFTypeSize:
			for (pChar=Buffer; *pStr; pStr++)
				switch (*pStr)
				{
				case L'0':
				case L'1':
				case L'2':
				case L'3':
				case L'4':
				case L'5':
				case L'6':
				case L'7':
				case L'8':
				case L'9':
					*(pChar++) = *pStr;

				case L'.':
				case L',':
				case L'\'':
					break;

				default:
					goto Abort;
				}

Abort:
			*pChar = L'\0';

			if (swscanf_s(Buffer, L"%I64d", &VData.INT64)==1)
			{
				VData.IsNull = FALSE;

				while (*pStr==L' ')
					pStr++;

				if ((_wcsicmp(pStr, L"KB")==0) || (_wcsicmp(pStr, L"K")==0))
				{
					VData.INT64 *= 1024;
				}
				else
					if ((_wcsicmp(pStr, L"MB")==0) || (_wcsicmp(pStr, L"M")==0))
					{
						VData.INT64 *= 1024*1024;
					}
					else
						if ((_wcsicmp(pStr, L"GB")==0) || (_wcsicmp(pStr, L"G")==0))
						{
							VData.INT64 *= 1024*1024*1024;
						}
			}

			break;

		case LFTypeFraction:
			if (swscanf_s(pStr, L"%u/%u", &VData.Fraction.Num, &VData.Fraction.Denum)==2)
				VData.IsNull = FALSE;

			break;

		case LFTypeDouble:
		case LFTypeMegapixel:
			if (swscanf_s(pStr, L"%lf", &VData.Double)==1)
				VData.IsNull = FALSE;

			break;

		case LFTypeGeoCoordinates:
			if (swscanf_s(pStr, L"%i?%i\'%i\"%c, %i?%i\'%i\"%c", &LatDeg, &LatMin, &LatSec, &LatCh, 1, &LonDeg, &LonMin, &LonSec, &LonCh, 1)==8)
				if (((LatCh==L'N') || (LatCh==L'S')) && ((LonCh==L'W') || (LonCh==L'E')))
				{
					VData.GeoCoordinates.Latitude = abs(LatDeg)+(abs(LatMin)/60.0)+(abs(LatSec)/3600.0);

					if (LatCh==L'N')
						VData.GeoCoordinates.Latitude -= VData.GeoCoordinates.Latitude;

					if ((VData.GeoCoordinates.Latitude<180.0) || (VData.GeoCoordinates.Latitude>180.0))
						VData.GeoCoordinates.Latitude = 0.0;

					VData.GeoCoordinates.Longitude = abs(LonDeg)+(abs(LonMin)/60.0)+(abs(LonSec)/3600.0);

					if (LonCh==L'W')
						VData.GeoCoordinates.Longitude -= VData.GeoCoordinates.Longitude;

					if ((VData.GeoCoordinates.Longitude<-180.0) || (VData.GeoCoordinates.Longitude>180.0))
						VData.GeoCoordinates.Longitude = 0.0;

					VData.IsNull = FALSE;
				}

			break;

		case LFTypeTime:
			ZeroMemory(&stLocal, sizeof(stLocal));

			switch (swscanf_s(pStr, L"%u%c%u%c%u", &Date1, &DateCh1, 1, &Date2, &DateCh2, 1, &Date3))
			{
			case 1:
			case 2:
				if ((Date1>1600) && (Date1<0x10000))
				{
					stLocal.wYear = (WORD)Date1;
					stLocal.wMonth = stLocal.wDay = 1;

					TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
					SystemTimeToFileTime(&stUTC, &VData.Time);
					VData.IsNull = !(VData.Time.dwHighDateTime | VData.Time.dwLowDateTime);
				}

				break;

			case 3:
			case 4:
				if ((Date1>=0) && (Date1<=12) && (Date2>1600) && (Date2<0x10000))
				{
					const UINT Temp = Date1;
					Date1 = Date2;
					Date2 = Temp;
				}

				if ((Date1>1600) && (Date1<0x10000) && (Date2>=0) && (Date2<=12))

				{
					stLocal.wYear = (WORD)Date1;
					stLocal.wMonth = (WORD)Date2;
					stLocal.wDay = 1;

					TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
					SystemTimeToFileTime(&stUTC, &VData.Time);

					VData.IsNull = !(VData.Time.dwHighDateTime || VData.Time.dwLowDateTime);
				}

				break;

			case 5:
				if ((DateCh1==DateCh2) && (Date1>0) && (Date2>0) && (Date3>0))
				{
					YearFirst = (Date1>1600) && (Date1<0x10000);
					if (YearFirst)
					{
						if ((DateCh1=='/') || ((Date2>12) && (Date3<=12)))
						{
							const UINT Temp = Date2;
							Date2 = Date3;
							Date3 = Temp;
						}

						if ((Date2<=12) && (Date3<=31))
						{
							stLocal.wYear = (WORD)Date1;
							stLocal.wMonth = (WORD)Date2;
							stLocal.wDay = (WORD)Date3;

							TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
							SystemTimeToFileTime(&stUTC, &VData.Time);

							VData.IsNull = !(VData.Time.dwHighDateTime || VData.Time.dwLowDateTime);
						}
					}
					else
						if ((Date3>1600) && (Date3<0x10000))
						{
							if ((DateCh1=='/') || ((Date1>12) && (Date2<=12)))
							{
								const UINT Temp = Date1;
								Date1 = Date2;
								Date2 = Temp;
							}

							if ((Date1<=31) && (Date2<=12))
							{
								stLocal.wYear = (WORD)Date3;
								stLocal.wMonth = (WORD)Date2;
								stLocal.wDay = (WORD)Date1;

								TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
								SystemTimeToFileTime(&stUTC, &VData.Time);

								VData.IsNull = !(VData.Time.dwHighDateTime || VData.Time.dwLowDateTime);
							}
						}
				}
			}

			break;

		case LFTypeBitrate:
			if (swscanf_s(pStr, L"%u", &VData.Bitrate)==1)
			{
				VData.Bitrate *= 1000;
				VData.IsNull = FALSE;
			}

			break;

		case LFTypeDuration:
			if (swscanf_s(pStr, L"%u:%u:%u", &Hour, &Min, &Sec)==3)
			{
				if ((Min<60) && (Sec<60))
				{
					VData.Duration = 1000*(Hour*3600+Min*60+Sec);
					VData.IsNull = FALSE;
				}
			}
			else
				if (swscanf_s(pStr, L"%u:%u", &Min, &Sec)==2)
				{
					if ((Min<60) && (Sec<60))
					{
						VData.Duration = 1000*(Min*60+Sec);
						VData.IsNull = FALSE;
					}
				}

			break;

		case LFTypeApplication:
			for (UINT a=1; a<APPLICATIONCOUNT; a++)
				if (_wcsicmp(pStr, ApplicationRegistry[a].Name)==0)
				{
					VData.Application = ApplicationRegistry[a].ApplicationID;
					VData.IsNull = FALSE;

					break;
				}

			break;
		}
	}
}

LFCORE_API INT LFCompareVariantData(const LFVariantData& VData1, const LFVariantData& VData2)
{
	if (VData1.IsNull && VData2.IsNull)
		return 0;

	if (VData1.IsNull)
		return -1;

	if (VData2.IsNull)
		return 1;

	if (VData1.Type!=VData2.Type)
		return VData1.Type-VData2.Type;

	return CompareValues(VData1.Type, &VData1.Value, &VData2.Value);
}

LFCORE_API void LFGetAttributeVariantData(const LFItemDescriptor* pItemDescriptor, LFVariantData& VData)
{
	assert(pItemDescriptor);

	if (pItemDescriptor->AttributeValues[VData.Attr])
	{
		assert(VData.Attr<LFAttributeCount);
		assert(VData.Type==AttrProperties[VData.Attr].Type);
		assert(VData.Type<LFTypeCount);

		switch (AttrProperties[VData.Attr].Type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(VData.UnicodeString, 256, (LPCWSTR)pItemDescriptor->AttributeValues[VData.Attr]);
			break;

		case LFTypeAnsiString:
			strcpy_s(VData.AnsiString, 256, (LPCSTR)pItemDescriptor->AttributeValues[VData.Attr]);
			break;

		case LFTypeIATACode:
			strcpy_s(VData.IATACode, 4, (LPCSTR)pItemDescriptor->AttributeValues[VData.Attr]);
			break;

		case LFTypeColor:
			VData.Color = *((BYTE*)pItemDescriptor->AttributeValues[VData.Attr]);

			assert(VData.Color<LFItemColorCount);
			VData.ColorSet = (1 << VData.Color);

			break;

		default:
			memcpy(&VData.Value, pItemDescriptor->AttributeValues[VData.Attr], TypeProperties[AttrProperties[VData.Attr].Type].Size);
		}

		VData.IsNull = FALSE;
	}
	else
	{
		LFClearVariantData(VData);
	}
}

LFCORE_API void LFGetAttributeVariantDataEx(const LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr, LFVariantData& VData)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);

	VData.Attr = Attr;
	VData.Type = AttrProperties[Attr].Type;

	LFGetAttributeVariantData(pItemDescriptor, VData);
}

LFCORE_API void LFSetAttributeVariantData(LFItemDescriptor* pItemDescriptor, const LFVariantData& VData)
{
	assert(pItemDescriptor);
	assert(VData.Attr<LFAttributeCount);
	assert(VData.Type==AttrProperties[VData.Attr].Type);
	assert(VData.Type<LFTypeCount);

	SetAttribute(pItemDescriptor, VData.Attr, &VData.Value);
}

LFCORE_API BOOL LFIsNullAttribute(const LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);

	return IsNullValue(AttrProperties[Attr].Type, pItemDescriptor->AttributeValues[Attr]);
}

LFCORE_API void LFSanitizeUnicodeArray(LPWSTR pStr, SIZE_T cCount)
{
	typedef std::pair<std::wstring, BOOL> TagItem;
	typedef stdext::hash_map<std::wstring, TagItem> Hashtags;
	Hashtags Tags;

	WCHAR Hashtag[259];
	LPCWSTR pHashtagArray = pStr;
	while (GetNextHashtag(&pHashtagArray, Hashtag, 256))
	{
		std::wstring Key(Hashtag);
		transform(Key.begin(), Key.end(), Key.begin(), towlower);

		Hashtags::iterator Location = Tags.find(Key);
		if (Location==Tags.end())
		{
			Tags[Key] = TagItem(Hashtag, FALSE);
		}
		else
			if (!Location->second.second)
				if (Location->second.first.compare(Hashtag)!=0)
					Location->second.second = TRUE;
	}

	pStr[0] = L'\0';
	for (Hashtags::iterator it=Tags.begin(); it!=Tags.end(); it++)
	{
		wcscpy_s(Hashtag, 259, pStr[0]!=L'\0' ? L" " : L"");

		if (it->second.first.find_first_of(L" .,:;?!|")!=std::wstring::npos)
		{
			wcscat_s(Hashtag, 259, L"\"");
			wcscat_s(Hashtag, 259, it->second.first.c_str());
			wcscat_s(Hashtag, 259, L"\"");
		}
		else
		{
			wcscat_s(Hashtag, 259, it->second.first.c_str());
		}

		if (wcslen(pStr)+wcslen(Hashtag)<=255)
		{
			if (it->second.second)
			{
				BOOL First = TRUE;
				for (WCHAR* pChar=Hashtag; *pChar; pChar++)
					switch (*pChar)
					{
					case L' ':
					case L'.':
					case L',':
					case L':':
					case L';':
					case L'?':
					case L'!':
					case L'|':
					case L'-':
					case L'"':
						First = TRUE;
						break;

					default:
						*pChar = First ? (WCHAR)toupper(*pChar) : (WCHAR)tolower(*pChar);
						First = FALSE;
					}
			}

			wcscat_s(pStr, cCount, Hashtag);
		}
	}
}
