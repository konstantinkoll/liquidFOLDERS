
#include "stdafx.h"
#include "ID3.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "TableApplications.h"
#include "TableAttributes.h"
#include <algorithm>
#include <assert.h>
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
		return (*(LPCWSTR)pValue==L'\0');

	case LFTypeAnsiString:
	case LFTypeIATACode:
		return (*(LPCSTR)pValue=='\0');

	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeBitrate:
	case LFTypeDuration:
	case LFTypeGenre:
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

INT CompareValues(UINT Type, LPCVOID pValue1, LPCVOID pValue2, BOOL CaseSensitive)
{
	assert(Type<LFTypeCount);
	assert(pValue1);
	assert(pValue2);

	UINT UInt1;
	UINT UInt2;
	DOUBLE Double1;
	DOUBLE Double2;

	switch (Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return CaseSensitive ? wcscmp((LPCWSTR)pValue1, (LPCWSTR)pValue2) : _wcsicmp((LPCWSTR)pValue1, (LPCWSTR)pValue2);

	case LFTypeAnsiString:
	case LFTypeIATACode:
		return CaseSensitive ? strcmp((LPCSTR)pValue1, (LPCSTR)pValue2) : _stricmp((LPCSTR)pValue1, (LPCSTR)pValue2);

	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeGenre:
		return *(UINT*)pValue1==*(UINT*)pValue2 ? 0 : *(UINT*)pValue1<*(UINT*)pValue2 ? -1 : 1;

	case LFTypeRating:
	case LFTypeColor:
		return (INT)(*(BYTE*)pValue1)-(INT)(*(BYTE*)pValue2);

	case LFTypeSize:
		return *(INT64*)pValue1==*(INT64*)pValue2 ? 0 : *(INT64*)pValue1<*(INT64*)pValue2 ? -1 : 1;

	case LFTypeDouble:
		Double1 = (DOUBLE)((INT)(*(DOUBLE*)pValue1*100.0))/100.0;
		Double2 = (DOUBLE)((INT)(*(DOUBLE*)pValue2*100.0))/100.0;

		return Double1==Double2 ? 0 : Double1<Double2 ? -1 : 1;

	case LFTypeGeoCoordinates:
		if (((LFGeoCoordinates*)pValue1)->Latitude==((LFGeoCoordinates*)pValue2)->Latitude)
		{
			return ((LFGeoCoordinates*)pValue1)->Longitude==((LFGeoCoordinates*)pValue2)->Longitude ? 0 : ((LFGeoCoordinates*)pValue1)->Longitude<((LFGeoCoordinates*)pValue2)->Longitude ? -1 : 1;
		}
		else
		{
			return ((LFGeoCoordinates*)pValue1)->Latitude<((LFGeoCoordinates*)pValue2)->Latitude ? -1 : 1;
		}

	case LFTypeTime:
		return CompareFileTime((FILETIME*)pValue1, (FILETIME*)pValue2);

	case LFTypeBitrate:
	case LFTypeDuration:
		UInt1 = *(UINT*)pValue1/1000;
		UInt2 = *(UINT*)pValue2/1000;

		return UInt1==UInt2 ? 0 : UInt1<UInt2 ? -1 : 1;

	case LFTypeMegapixel:
		Double1 = (DOUBLE)((INT)(*(DOUBLE*)pValue1*10.0))/10.0;
		Double2 = (DOUBLE)((INT)(*(DOUBLE*)pValue2*10.0))/10.0;

		return (Double1==Double2) ? 0 : Double1<Double2 ? -1 : 1;

	case LFTypeApplication:
		UInt1 = *(BYTE*)pValue1;
		UInt2 = *(BYTE*)pValue2;

		if ((UInt1>LFApplicationCount) && (UInt2>LFApplicationCount))
			return 0;

		if (UInt1>LFApplicationCount)
			return -1;

		if (UInt2>LFApplicationCount)
			return -1;

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
				wcscat_s(pStr, cCount, L"½");

			return;

		case LFTypeColor:
			wcscpy_s(pStr, cCount, *((BYTE*)pValue) ? ItemColorNames[*((BYTE*)pValue)-1] : L"");
			return;

		case LFTypeUINT:
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
			LFTimeToString(*((FILETIME*)pValue), pStr, cCount);
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

LFCORE_API void LFFourCCToString(const UINT c, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (cCount<5)
	{
		*pStr = L'\0';
	}
	else
	{
		pStr[0] = c & 0xFF;
		pStr[1] = (c>>8) & 0xFF;
		pStr[2] = (c>>16) & 0xFF;
		pStr[3] = c>>24;
		pStr[4] = '\0';
	}
}

LFCORE_API void LFUINTToString(const UINT u, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (u==0)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u", u);
	}
}

LFCORE_API void LFSizeToString(const INT64 s, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	StrFormatByteSize(s, pStr, (UINT)cCount);
}

LFCORE_API void LFFractionToString(const LFFraction f, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if ((f.Num==0) || (f.Denum==0))
	{
		pStr[0] = L'\0';
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u/%u", f.Num, f.Denum);
	}
}

LFCORE_API void LFDoubleToString(const DOUBLE d, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	swprintf_s(pStr, cCount, L"%.2lf", d);
}

LFCORE_API void LFGeoCoordinateToString(const DOUBLE c, LPWSTR pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero)
{
	assert(pStr);

	swprintf_s(pStr, cCount, FillZero ? L"%03u°%02u\'%02u\"%c" : L"%u°%u\'%u\"%c",
		(UINT)(fabs(c)+ROUNDOFF),
		(UINT)GetMinutes(c),
		(UINT)(GetSeconds(c)+0.5),
		c>0 ? IsLatitude ? L'S' : L'E' : IsLatitude ? L'N' : L'W');
}

LFCORE_API void LFGeoCoordinatesToString(const LFGeoCoordinates& c, LPWSTR pStr, SIZE_T cCount, BOOL FillZero)
{
	assert(pStr);

	if ((c.Latitude==0) && (c.Longitude==0))
	{
		*pStr = L'\0';
	}
	else
	{
		WCHAR tmpStr[32];
		LFGeoCoordinateToString(c.Longitude, tmpStr, 32, FALSE, FillZero);

		LFGeoCoordinateToString(c.Latitude, pStr, cCount, TRUE, FillZero);
		wcscat_s(pStr, cCount, L", ");
		wcscat_s(pStr, cCount, tmpStr);
	}
}

LFCORE_API void LFTimeToString(const FILETIME t, LPWSTR pStr, SIZE_T cCount, BOOL IncludeTime)
{
	assert(pStr);

	*pStr = L'\0';

	if ((t.dwHighDateTime) || (t.dwLowDateTime))
	{
		SYSTEMTIME stUTC;
		SYSTEMTIME stLocal;
		FileTimeToSystemTime(&t, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

		INT Size = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal, NULL, pStr, (INT)cCount);

		if (IncludeTime && ((stLocal.wHour) || (stLocal.wMinute) || (stLocal.wSecond)))
		{
			pStr[Size-1] = L' ';

			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &stLocal, NULL, &pStr[Size], (INT)cCount-Size);
		}
	}
}

LFCORE_API void LFBitrateToString(const UINT r, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (r==0)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf_s(pStr, cCount, L"%u kBit/s", (r+500)/1000);
	}
}

LFCORE_API void LFDurationToString(UINT d, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	d = (d+500)/1000;

	if (d==0)
	{
		*pStr = L'\0';
	}
	else
		if (d/3600)
		{
			swprintf_s(pStr, cCount, L"%02d:%02d:%02d", d/3600, (d/60)%60, d%60);
		}
		else
		{
			swprintf_s(pStr, cCount, L"%02d:%02d", d/60, d%60);
		}
}

LFCORE_API void LFMegapixelToString(const DOUBLE d, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	swprintf_s(pStr, cCount, L"%.1lf Megapixel", d);
}

LFCORE_API void LFAttributeToString(const LFItemDescriptor* pItemDescriptor, UINT Attr, LPWSTR pStr, SIZE_T cCount)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);
	assert(AttrProperties[Attr].Type<LFTypeCount);

	ToString(pItemDescriptor->AttributeValues[Attr], AttrProperties[Attr].Type, pStr, cCount);
}


// LFVariantData
//

LFCORE_API void LFInitVariantData(LFVariantData& pValue, UINT Attr)
{
	pValue.Attr = Attr;
	pValue.Type = (Attr<LFAttributeCount) ? AttrProperties[Attr].Type : LFTypeUnicodeString;

	LFClearVariantData(pValue);
}

LFCORE_API void LFClearVariantData(LFVariantData& pValue)
{
	assert(pValue.Type<LFTypeCount);

	pValue.IsNull = TRUE;

	switch (pValue.Type)
	{
	case LFTypeDouble:
		pValue.Double = 0;
		break;

	case LFTypeGeoCoordinates:
		pValue.GeoCoordinates.Latitude = pValue.GeoCoordinates.Longitude = 0;
		break;

	case LFTypeMegapixel:
		pValue.Double = -1;
		break;

	default:
		pValue.INT64 = 0;
	}
}

LFCORE_API BOOL LFIsNullVariantData(const LFVariantData& Value)
{
	return Value.IsNull ? TRUE : IsNullValue(Value.Type, &Value.Value);
}

LFCORE_API void LFVariantDataToString(const LFVariantData& Value, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (Value.IsNull)
	{
		*pStr = L'\0';
	}
	else
	{
		ToString(&Value.Value, Value.Type, pStr, cCount);
	}
}

LFCORE_API void LFVariantDataFromString(LFVariantData& pValue, LPCWSTR pStr)
{
	LFClearVariantData(pValue);

	if (pStr)
	{
		SIZE_T Size = wcslen(pStr);

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

		switch (pValue.Type)
		{
		case LFTypeUnicodeString:
			wcsncpy_s(pValue.UnicodeString, 256, pStr, _TRUNCATE);
			pValue.IsNull = FALSE;

			break;

		case LFTypeUnicodeArray:
			wcsncpy_s(pValue.UnicodeArray, 256, pStr, _TRUNCATE);
			LFSanitizeUnicodeArray(pValue.UnicodeArray, 256);
			pValue.IsNull = FALSE;

			break;

		case LFTypeAnsiString:
		case LFTypeIATACode:
			WideCharToMultiByte(CP_ACP, 0, pStr, -1, pValue.AnsiString, 256, NULL, NULL);
			pValue.IsNull = FALSE;

			if (pValue.Type==LFTypeIATACode)
			{
				pValue.IATAString[0] = (CHAR)toupper(pValue.IATAString[0]);
				pValue.IATAString[1] = (CHAR)toupper(pValue.IATAString[1]);
				pValue.IATAString[2] = (CHAR)toupper(pValue.IATAString[2]);
			}

			break;

		case LFTypeRating:
			if (wcscmp(pStr, L"½")==0)
			{
				pValue.Rating = 1;
				pValue.IsNull = FALSE;

				return;
			}

			if ((pStr[0]>=L'0') && (pStr[0]<=L'5'))
				if (Size<=2)
				{
					pValue.Rating = (BYTE)((pStr[0]-L'0')*2);

					if (Size==2)
						if (pStr[1]==L'½')
							pValue.Rating++;

					pValue.IsNull = FALSE;

					return;
				}

			if ((Size>=1) && (Size<=5))
			{
				BOOL Same = TRUE;
				for (UINT a=1; a<Size; a++)
					Same &= (pStr[a]==pStr[a-1]);

				if (Same)
				{
					pValue.Rating = (BYTE)(Size*2);
					pValue.IsNull = FALSE;

					return;
				}
			}

			break;

		case LFTypeUINT:
			if (swscanf_s(pStr, L"%u", &pValue.UINT32)==1)
				pValue.IsNull = FALSE;

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

			if (swscanf_s(Buffer, L"%I64d", &pValue.INT64)==1)
			{
				pValue.IsNull = FALSE;

				while (*pStr==L' ')
					pStr++;

				if ((_wcsicmp(pStr, L"KB")==0) || (_wcsicmp(pStr, L"K")==0))
				{
					pValue.INT64 *= 1024;
				}
				else
					if ((_wcsicmp(pStr, L"MB")==0) || (_wcsicmp(pStr, L"M")==0))
					{
						pValue.INT64 *= 1024*1024;
					}
					else
						if ((_wcsicmp(pStr, L"GB")==0) || (_wcsicmp(pStr, L"G")==0))
						{
							pValue.INT64 *= 1024*1024*1024;
						}
			}

			break;

		case LFTypeFraction:
			if (swscanf_s(pStr, L"%u/%u", &pValue.Fraction.Num, &pValue.Fraction.Denum)==2)
				pValue.IsNull = FALSE;

			break;

		case LFTypeDouble:
		case LFTypeMegapixel:
			if (swscanf_s(pStr, L"%lf", &pValue.Double)==1)
				pValue.IsNull = FALSE;

			break;

		case LFTypeGeoCoordinates:
			if (swscanf_s(pStr, L"%i°%i\'%i\"%c, %i°%i\'%i\"%c", &LatDeg, &LatMin, &LatSec, &LatCh, 1, &LonDeg, &LonMin, &LonSec, &LonCh, 1)==8)
				if (((LatCh==L'N') || (LatCh==L'S')) && ((LonCh==L'W') || (LonCh==L'E')))
				{
					pValue.GeoCoordinates.Latitude = abs(LatDeg)+(abs(LatMin)/60.0)+(abs(LatSec)/3600.0);
					if (LatCh==L'N')
						pValue.GeoCoordinates.Latitude -= pValue.GeoCoordinates.Latitude;
					if ((pValue.GeoCoordinates.Latitude<180.0) || (pValue.GeoCoordinates.Latitude>180.0))
						pValue.GeoCoordinates.Latitude = 0.0;

					pValue.GeoCoordinates.Longitude = abs(LonDeg)+(abs(LonMin)/60.0)+(abs(LonSec)/3600.0);
					if (LonCh==L'W')
						pValue.GeoCoordinates.Longitude -= pValue.GeoCoordinates.Longitude;
					if ((pValue.GeoCoordinates.Longitude<-180.0) || (pValue.GeoCoordinates.Longitude>180.0))
						pValue.GeoCoordinates.Longitude = 0.0;

					pValue.IsNull = FALSE;
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
					SystemTimeToFileTime(&stUTC, &pValue.Time);
					pValue.IsNull = !((pValue.Time.dwHighDateTime) || (pValue.Time.dwLowDateTime));
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
					SystemTimeToFileTime(&stUTC, &pValue.Time);
					pValue.IsNull = !((pValue.Time.dwHighDateTime) || (pValue.Time.dwLowDateTime));
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
							SystemTimeToFileTime(&stUTC, &pValue.Time);

							pValue.IsNull = !((pValue.Time.dwHighDateTime) || (pValue.Time.dwLowDateTime));
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
								SystemTimeToFileTime(&stUTC, &pValue.Time);

								pValue.IsNull = !((pValue.Time.dwHighDateTime) || (pValue.Time.dwLowDateTime));
							}
						}
				}
			}

			break;

		case LFTypeBitrate:
			if (swscanf_s(pStr, L"%u", &pValue.Bitrate)==1)
			{
				pValue.Bitrate *= 1000;
				pValue.IsNull = FALSE;
			}

			break;

		case LFTypeDuration:
			if (swscanf_s(pStr, L"%u:%u:%u", &Hour, &Min, &Sec)==3)
			{
				if ((Min<60) && (Sec<60))
				{
					pValue.Duration = 1000*(Hour*3600+Min*60+Sec);
					pValue.IsNull = FALSE;
				}
			}
			else
				if (swscanf_s(pStr, L"%u:%u", &Min, &Sec)==2)
				{
					if ((Min<60) && (Sec<60))
					{
						pValue.Duration = 1000*(Min*60+Sec);
						pValue.IsNull = FALSE;
					}
				}

			break;
		}
	}
}

LFCORE_API INT LFCompareVariantData(LFVariantData& Value1, LFVariantData& Value2)
{
	if (Value1.IsNull && Value2.IsNull)
		return 0;

	if (Value1.IsNull)
		return -1;

	if (Value2.IsNull)
		return 1;

	if (Value1.Type!=Value2.Type)
		return Value1.Type-Value2.Type;

	return CompareValues(Value1.Type, &Value1.Value, &Value2.Value);
}

LFCORE_API void LFGetAttributeVariantData(const LFItemDescriptor* pItemDescriptor, LFVariantData& Value)
{
	assert(pItemDescriptor);

	if (pItemDescriptor->AttributeValues[Value.Attr])
	{
		assert(Value.Attr<LFAttributeCount);
		assert(Value.Type==AttrProperties[Value.Attr].Type);
		assert(Value.Type<LFTypeCount);

		switch (AttrProperties[Value.Attr].Type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(Value.UnicodeString, 256, (LPCWSTR)pItemDescriptor->AttributeValues[Value.Attr]);
			break;

		case LFTypeAnsiString:
			strcpy_s(Value.AnsiString, 256, (LPCSTR)pItemDescriptor->AttributeValues[Value.Attr]);
			break;

		case LFTypeIATACode:
			strcpy_s(Value.IATAString, 4, (LPCSTR)pItemDescriptor->AttributeValues[Value.Attr]);
			break;

		case LFTypeColor:
			Value.Color = *((BYTE*)pItemDescriptor->AttributeValues[Value.Attr]);

			assert(Value.Color<LFItemColorCount);
			Value.ColorSet = (1 << Value.Color);

			break;

		default:
			memcpy(&Value.Value, pItemDescriptor->AttributeValues[Value.Attr], TypeProperties[AttrProperties[Value.Attr].Type].Size);
		}

		Value.IsNull = FALSE;
	}
	else
	{
		LFClearVariantData(Value);
	}
}

LFCORE_API void LFGetAttributeVariantDataEx(const LFItemDescriptor* pItemDescriptor, UINT Attr, LFVariantData& Value)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);

	Value.Attr = Attr;
	Value.Type = AttrProperties[Attr].Type;

	LFGetAttributeVariantData(pItemDescriptor, Value);
}

LFCORE_API void LFSetAttributeVariantData(LFItemDescriptor* pItemDescriptor, const LFVariantData& Value)
{
	assert(pItemDescriptor);
	assert(Value.Attr<LFAttributeCount);
	assert(Value.Type==AttrProperties[Value.Attr].Type);
	assert(Value.Type<LFTypeCount);

	SetAttribute(pItemDescriptor, Value.Attr, &Value.Value);
}

LFCORE_API BOOL LFIsNullAttribute(const LFItemDescriptor* pItemDescriptor, UINT Attr)
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
