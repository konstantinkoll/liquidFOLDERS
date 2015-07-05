
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include <algorithm>
#include <assert.h>
#include <hash_map>
#include <math.h>
#include <shlwapi.h>


extern const BYTE AttrTypes[];


// Interne Methoden
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

BOOL IsNullValue(UINT Type, void* v)
{
	if (!v)
		return TRUE;

	assert(Type<LFTypeCount);

	switch (Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (*(WCHAR*)v==L'\0');

	case LFTypeAnsiString:
		return (*(CHAR*)v=='\0');

	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeDuration:
	case LFTypeBitrate:
		return (*(UINT*)v)==0;

	case LFTypeRating:
		return (*(BYTE*)v)==0;

	case LFTypeSize:
	case LFTypeTime:
		return (*(INT64*)v)==0;

	case LFTypeFraction:
		return (((LFFraction*)v)->Num==0) || (((LFFraction*)v)->Denum==0);

	case LFTypeDouble:
	case LFTypeMegapixel:
		return (*(DOUBLE*)v)==-1;

	case LFTypeGeoCoordinates:
		return (((LFGeoCoordinates*)v)->Latitude==0) && (((LFGeoCoordinates*)v)->Longitude==0);
	}

	return FALSE;
}

INT CompareValues(UINT Type, void* v1, void* v2, BOOL CaseSensitive)
{
	assert(Type<LFTypeCount);
	assert(v1);
	assert(v2);

	UINT u1;
	UINT u2;
	DOUBLE d1;
	DOUBLE d2;

	switch (Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return CaseSensitive ? wcscmp((WCHAR*)v1, (WCHAR*)v2) : _wcsicmp((WCHAR*)v1, (WCHAR*)v2);

	case LFTypeAnsiString:
		return CaseSensitive ? strcmp((CHAR*)v1, (CHAR*)v2) : _stricmp((CHAR*)v1, (CHAR*)v2);

	case LFTypeFourCC:
	case LFTypeUINT:
		return *(UINT*)v1==*(UINT*)v2 ? 0 : *(UINT*)v1<*(UINT*)v2 ? -1 : 1;

	case LFTypeRating:
		return (INT)(*(BYTE*)v1)-(INT)(*(BYTE*)v2);

	case LFTypeSize:
		return *(INT64*)v1==*(INT64*)v2 ? 0 : *(INT64*)v1<*(INT64*)v2 ? -1 : 1;

	case LFTypeDouble:
		d1 = (DOUBLE)((INT)(*(DOUBLE*)v1*100.0))/100.0;
		d2 = (DOUBLE)((INT)(*(DOUBLE*)v2*100.0))/100.0;

		return d1==d2 ? 0 : d1<d2 ? -1 : 1;

	case LFTypeTime:
		return CompareFileTime((FILETIME*)v1, (FILETIME*)v2);

	case LFTypeBitrate:
	case LFTypeDuration:
		u1 = *(UINT*)v1/1000;
		u2 = *(UINT*)v2/1000;

		return u1==u2 ? 0 : u1<u2 ? -1 : 1;

	case LFTypeMegapixel:
		d1 = (DOUBLE)((INT)(*(DOUBLE*)v1*10.0))/10.0;
		d2 = (DOUBLE)((INT)(*(DOUBLE*)v2*10.0))/10.0;

		return d1==d2 ? 0 : d1<d2 ? -1 : 1;
	}

	return 0;
}

void ToString(void* v, UINT Type, WCHAR* pStr, SIZE_T cCount)
{
	assert(Type<LFTypeCount);
	assert(pStr);

	if (v)
		switch (Type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(pStr, cCount, (WCHAR*)v);
			return;

		case LFTypeAnsiString:
			MultiByteToWideChar(CP_ACP, 0, (CHAR*)v, -1, pStr, (INT)cCount);
			return;

		case LFTypeFourCC:
			LFFourCCToString(*((UINT*)v), pStr, cCount);
			return;

		case LFTypeRating:
			assert(*((BYTE*)v)<=LFMaxRating);

			if (*((BYTE*)v)!=1)
			{
				swprintf_s(pStr, cCount, L"%u", (UINT)(*((BYTE*)v)/2));
			}
			else
			{
				pStr[0] = L'\0';
			}

			if (*((BYTE*)v)%2)
				wcscat_s(pStr, cCount, L"½");

			return;

		case LFTypeUINT:
			LFUINTToString(*((UINT*)v), pStr, cCount);
			return;

		case LFTypeSize:
			LFSizeToString(*((INT64*)v), pStr, cCount);
			return;

		case LFTypeFraction:
			LFFractionToString(*((LFFraction*)v), pStr, cCount);
			return;

		case LFTypeDouble:
			LFDoubleToString(*((DOUBLE*)v), pStr, cCount);
			return;

		case LFTypeFlags:
			if (cCount>=5)
			{
				pStr[0] = (*((UINT*)v) & LFFlagLink) ? 'L' : '-';
				pStr[1] = (*((UINT*)v) & LFFlagNew) ? 'N' : '-';
				pStr[2] = (*((UINT*)v) & LFFlagTrash) ? 'T' : '-';
				pStr[3] = (*((UINT*)v) & LFFlagMissing) ? 'M' : '-';
				pStr[4] = '\0';
			}

			return;

		case LFTypeGeoCoordinates:
			LFGeoCoordinatesToString(*((LFGeoCoordinates*)v), pStr, cCount, FALSE);
			return;

		case LFTypeTime:
			LFTimeToString(*((FILETIME*)v), pStr, cCount);
			return;

		case LFTypeDuration:
			LFDurationToString(*((UINT*)v), pStr, cCount);
			return;

		case LFTypeBitrate:
			LFBitrateToString(*((UINT*)v), pStr, cCount);
			return;

		case LFTypeMegapixel:
			LFMegapixelToString(*((DOUBLE*)v), pStr, cCount);
			return;
		}

	*pStr = L'\0';
}

BOOL GetNextTag(WCHAR** ppUnicodeArray, WCHAR* Tag, SIZE_T cCount)
{
	WCHAR* Start = NULL;
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
				wcsncpy_s(Tag, cCount, Start, ((*ppUnicodeArray)++)-Start);

				return TRUE;
			}

			break;

		case L'"':
			if (InQuotation)
			{
				if (Start)
				{
					wcsncpy_s(Tag, cCount, Start, ((*ppUnicodeArray)++)-Start);

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
		wcscpy_s(Tag, cCount, Start);

		return TRUE;
	}

	return FALSE;
}


// ToString
//

LFCORE_API void LFFourCCToString(const UINT c, WCHAR* pStr, SIZE_T cCount)
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

LFCORE_API void LFUINTToString(const UINT u, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	if (u==0)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf(pStr, cCount, L"%u", u);
	}
}

LFCORE_API void LFSizeToString(const INT64 i, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	StrFormatByteSize(i, pStr, (UINT)cCount);
}

LFCORE_API void LFFractionToString(const LFFraction frac, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	if ((frac.Num==0) || (frac.Denum==0))
	{
		pStr[0] = L'\0';
	}
	else
	{
		swprintf(pStr, cCount, L"%u/%u", frac.Num, frac.Denum);
	}
}

LFCORE_API void LFDoubleToString(const DOUBLE d, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	swprintf(pStr, cCount, L"%.2lf", d);
}

LFCORE_API void LFGeoCoordinateToString(const DOUBLE c, WCHAR* pStr, SIZE_T cCount, BOOL IsLatitude, BOOL FillZero)
{
	assert(pStr);

	swprintf(pStr, cCount, FillZero ? L"%03u°%02u\'%02u\"%c" : L"%u°%u\'%u\"%c",
		(UINT)(fabs(c)+ROUNDOFF),
		(UINT)GetMinutes(c),
		(UINT)(GetSeconds(c)+0.5),
		c>0 ? IsLatitude ? L'S' : L'E' : IsLatitude ? L'N' : L'W');
}

LFCORE_API void LFGeoCoordinatesToString(const LFGeoCoordinates c, WCHAR* pStr, SIZE_T cCount, BOOL FillZero)
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

LFCORE_API void LFTimeToString(const FILETIME t, WCHAR* pStr, SIZE_T cCount, BOOL IncludeTime)
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

LFCORE_API void LFDurationToString(UINT d, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	d = (d+500)/1000;

	if (d==0)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf(pStr, cCount, L"%02d:%02d:%02d", d/3600, (d/60)%60, d%60);
	}
}

LFCORE_API void LFBitrateToString(const UINT r, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	if (r==0)
	{
		*pStr = L'\0';
	}
	else
	{
		swprintf(pStr, cCount, L"%u kBit/s", (r+500)/1000);
	}
}

LFCORE_API void LFMegapixelToString(const DOUBLE d, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	swprintf(pStr, cCount, L"%.1lf Megapixel", d);
}

LFCORE_API void LFAttributeToString(LFItemDescriptor* i, UINT Attr, WCHAR* pStr, SIZE_T cCount)
{
	assert(i);
	assert(Attr<LFAttributeCount);
	assert(AttrTypes[Attr]<LFTypeCount);

	ToString(i->AttributeValues[Attr], AttrTypes[Attr], pStr, cCount);
}


// LFVariantData
//

LFCORE_API void LFInitVariantData(LFVariantData& v, UINT Attr)
{
	v.Attr = Attr;
	v.Type = (Attr<LFAttributeCount) ? AttrTypes[Attr] : LFTypeUnicodeString;

	LFClearVariantData(v);
}

LFCORE_API void LFClearVariantData(LFVariantData& v)
{
	assert(v.Type<LFTypeCount);

	v.IsNull = TRUE;

	switch (v.Type)
	{
	case LFTypeDouble:
		v.Double = 0;
		break;

	case LFTypeGeoCoordinates:
		v.GeoCoordinates.Latitude = v.GeoCoordinates.Longitude = 0;
		break;

	case LFTypeMegapixel:
		v.Double = -1;
		break;

	default:
		v.INT64 = 0;
	}
}

LFCORE_API BOOL LFIsNullVariantData(LFVariantData& v)
{
	return v.IsNull ? TRUE : IsNullValue(v.Type, &v.Value);
}

LFCORE_API void LFVariantDataToString(LFVariantData& v, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	if (v.IsNull)
	{
		*pStr = L'\0';
	}
	else
	{
		assert(v.Type<LFTypeCount);

		ToString(&v.Value, v.Type, pStr, cCount);
	}
}

LFCORE_API void LFVariantDataFromString(LFVariantData& v, WCHAR* pStr)
{
	LFClearVariantData(v);

	if (pStr)
	{
		SIZE_T sz = wcslen(pStr);

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

		switch (v.Type)
		{
		case LFTypeUnicodeString:
			wcscpy_s(v.UnicodeString, 256, pStr);
			v.IsNull = FALSE;
			break;

		case LFTypeUnicodeArray:
			wcscpy_s(v.UnicodeArray, 256, pStr);
			LFSanitizeUnicodeArray(v.UnicodeArray, 256);
			v.IsNull = FALSE;
			break;

		case LFTypeAnsiString:
			WideCharToMultiByte(CP_ACP, 0, pStr, -1, v.AnsiString, 256, NULL, NULL);
			v.IsNull = FALSE;
			break;

		case LFTypeRating:
			if (wcscmp(pStr, L"½")==0)
			{
				v.Rating = 1;
				v.IsNull = FALSE;
				return;
			}

			if ((pStr[0]>=L'0') && (pStr[0]<=L'5'))
				if (sz<=2)
				{
					v.Rating = (BYTE)((pStr[0]-L'0')*2);

					if (sz==2)
						if (pStr[1]==L'½')
							v.Rating++;

					v.IsNull = FALSE;
					return;
				}

			if ((sz>=1) && (sz<=5))
			{
				BOOL Same = TRUE;
				for (UINT a=1; a<sz; a++)
					Same &= (pStr[a]==pStr[a-1]);

				if (Same)
				{
					v.Rating = (BYTE)(sz*2);
					v.IsNull = FALSE;
					return;
				}
			}

			break;

		case LFTypeUINT:
			if (swscanf_s(pStr, L"%u", &v.UINT32)==1)
				v.IsNull = FALSE;

			break;

		case LFTypeFraction:
			if (swscanf_s(pStr, L"%u/%u", &v.Fraction.Num, &v.Fraction.Denum)==2)
				v.IsNull = FALSE;
			break;

		case LFTypeDouble:
		case LFTypeMegapixel:
			if (swscanf_s(pStr, L"%lf", &v.Double)==1)
				v.IsNull = FALSE;

			break;

		case LFTypeGeoCoordinates:
			if (swscanf_s(pStr, L"%i°%i\'%i\"%c, %i°%i\'%i\"%c", &LatDeg, &LatMin, &LatSec, &LatCh, 1, &LonDeg, &LonMin, &LonSec, &LonCh, 1)==8)
				if (((LatCh==L'N') || (LatCh==L'S')) && ((LonCh==L'W') || (LonCh==L'E')))
				{
					v.GeoCoordinates.Latitude = abs(LatDeg)+(abs(LatMin)/60.0)+(abs(LatSec)/3600.0);
					if (LatCh==L'N')
						v.GeoCoordinates.Latitude -= v.GeoCoordinates.Latitude;
					if ((v.GeoCoordinates.Latitude<180.0) || (v.GeoCoordinates.Latitude>180.0))
						v.GeoCoordinates.Latitude = 0.0;

					v.GeoCoordinates.Longitude = abs(LonDeg)+(abs(LonMin)/60.0)+(abs(LonSec)/3600.0);
					if (LonCh==L'W')
						v.GeoCoordinates.Longitude -= v.GeoCoordinates.Longitude;
					if ((v.GeoCoordinates.Longitude<-180.0) || (v.GeoCoordinates.Longitude>180.0))
						v.GeoCoordinates.Longitude = 0.0;

					v.IsNull = FALSE;
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
					SystemTimeToFileTime(&stUTC, &v.Time);
					v.IsNull = !((v.Time.dwHighDateTime) || (v.Time.dwLowDateTime));
				}

				break;

			case 3:
			case 4:
				if ((Date1>=0) && (Date1<=12) && (Date2>1600) && (Date2<0x10000))
				{
					UINT Temp = Date1;
					Date1 = Date2;
					Date2 = Temp;
				}

				if ((Date1>1600) && (Date1<0x10000) && (Date2>=0) && (Date2<=12))

				{
					stLocal.wYear = (WORD)Date1;
					stLocal.wMonth = (WORD)Date2;
					stLocal.wDay = 1;

					TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
					SystemTimeToFileTime(&stUTC, &v.Time);
					v.IsNull = !((v.Time.dwHighDateTime) || (v.Time.dwLowDateTime));
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
							UINT Temp = Date2;
							Date2 = Date3;
							Date3 = Temp;
						}

						if ((Date2<=12) && (Date3<=31))
						{
							stLocal.wYear = (WORD)Date1;
							stLocal.wMonth = (WORD)Date2;
							stLocal.wDay = (WORD)Date3;

							TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
							SystemTimeToFileTime(&stUTC, &v.Time);

							v.IsNull = !((v.Time.dwHighDateTime) || (v.Time.dwLowDateTime));
						}
					}
					else
						if ((Date3>1600) && (Date3<0x10000))
						{
							if ((DateCh1=='/') || ((Date1>12) && (Date2<=12)))
							{
								UINT Temp = Date1;
								Date1 = Date2;
								Date2 = Temp;
							}

							if ((Date1<=31) && (Date2<=12))
							{
								stLocal.wYear = (WORD)Date3;
								stLocal.wMonth = (WORD)Date2;
								stLocal.wDay = (WORD)Date1;

								TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
								SystemTimeToFileTime(&stUTC, &v.Time);

								v.IsNull = !((v.Time.dwHighDateTime) || (v.Time.dwLowDateTime));
							}
						}
				}
			}

			break;

		case LFTypeDuration:
			if (swscanf_s(pStr, L"%u:%u:%u", &Hour, &Min, &Sec)==3)
			{
				v.Duration = 1000*(Hour*3600+Min*60+Sec);
				v.IsNull = FALSE;
			}

			break;

		case LFTypeBitrate:
			if (swscanf_s(pStr, L"%u", &v.Bitrate)==1)
			{
				v.Bitrate *= 1000;
				v.IsNull = FALSE;
			}

			break;
		}
	}
}

LFCORE_API INT LFCompareVariantData(LFVariantData& v1, LFVariantData& v2)
{
	if (v1.IsNull && v2.IsNull)
		return 0;
	if (v1.IsNull)
		return -1;
	if (v2.IsNull)
		return 1;

	if (v1.Type!=v2.Type)
		return v1.Type-v2.Type;

	return CompareValues(v1.Type, &v1.Value, &v2.Value);
}

LFCORE_API void LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData& v)
{
	assert(i);

	if (i->AttributeValues[v.Attr])
	{
		assert(v.Attr<LFAttributeCount);
		assert(v.Type==AttrTypes[v.Attr]);
		assert(v.Type<LFTypeCount);

		memcpy(&v.Value, i->AttributeValues[v.Attr], GetAttributeSize(v.Attr, i->AttributeValues[v.Attr]));
		v.IsNull = FALSE;
	}
	else
	{
		LFClearVariantData(v);
	}
}

LFCORE_API void LFGetAttributeVariantDataEx(LFItemDescriptor* i, UINT Attr, LFVariantData& v)
{
	assert(i);

	v.Attr = Attr;
	v.Type = AttrTypes[Attr];

	LFGetAttributeVariantData(i, v);
}

LFCORE_API void LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData& v)
{
	assert(i);
	assert(v.Attr<LFAttributeCount);
	assert(v.Type==AttrTypes[v.Attr]);
	assert(v.Type<LFTypeCount);

	// Special treatment for flags
	if (v.Attr==LFAttrFlags)
	{
		v.Flags.Mask &= LFFlagArchive | LFFlagTrash;
		v.Flags.Flags &= v.Flags.Mask;

		i->CoreAttributes.Flags &= ~v.Flags.Mask;
		i->CoreAttributes.Flags |= v.Flags.Flags;
	}
	else
	{
		SetAttribute(i, v.Attr, &v.Value);
	}
}

LFCORE_API void LFSanitizeUnicodeArray(WCHAR* pBuffer, SIZE_T cCount)
{
	typedef std::pair<std::wstring, BOOL> TagItem;
	typedef stdext::hash_map<std::wstring, TagItem> Hashtags;
	Hashtags Tags;

	WCHAR Tag[259];
	WCHAR* Tagarray = pBuffer;
	while (GetNextTag(&Tagarray, Tag, 256))
	{
		std::wstring Key(Tag);
		transform(Key.begin(), Key.end(), Key.begin(), towlower);

		Hashtags::iterator Location = Tags.find(Key);
		if (Location==Tags.end())
		{
			Tags[Key] = TagItem(Tag, FALSE);
		}
		else
			if (!Location->second.second)
				if (Location->second.first.compare(Tag)!=0)
					Location->second.second = TRUE;
	}

	pBuffer[0] = L'\0';
	for (Hashtags::iterator it=Tags.begin(); it!=Tags.end(); it++)
	{
		wcscpy_s(Tag, 259, pBuffer[0]!=L'\0' ? L" " : L"");

		if (it->second.first.find_first_of(L" .,:;?!|")!=std::wstring::npos)
		{
			wcscat_s(Tag, 259, L"\"");
			wcscat_s(Tag, 259, it->second.first.c_str());
			wcscat_s(Tag, 259, L"\"");
		}
		else
		{
			wcscat_s(Tag, 259, it->second.first.c_str());
		}

		if (wcslen(pBuffer)+wcslen(Tag)<=255)
		{
			if (it->second.second)
			{
				BOOL First = TRUE;
				for (WCHAR* Ptr=Tag; *Ptr; Ptr++)
					switch (*Ptr)
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
						*Ptr = First ? (WCHAR)toupper(*Ptr) : (WCHAR)tolower(*Ptr);
						First = FALSE;
					}
			}

			wcscat_s(pBuffer, cCount, Tag);
		}
	}
}
