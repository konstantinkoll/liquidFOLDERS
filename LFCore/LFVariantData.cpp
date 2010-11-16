
#include "StdAfx.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "LFCore.h"
#include <assert.h>
#include <cmath>
#include <hash_map>
#include <shlwapi.h>
#include <wchar.h>


extern unsigned char AttrTypes[];

wchar_t RatingStrings[6] = L"\x2605\x2605\x2605\x2605\x2605";


// Conversion ToString
//

#define ROUNDOFF 0.00000001

inline double GetMinutes(double c)
{
	c = fabs(c)+ROUNDOFF;
	return (c-(double)(int)c)*60.0;
}

inline double GetSeconds(double c)
{
	c = fabs(c)*60.0+ROUNDOFF;
	return (c-(double)(int)c)*60.0;
}


LFCore_API void LFFourCCToString(const unsigned int c, wchar_t* str, size_t cCount)
{
	if (cCount>=5)
	{
		str[0] = c & 0xFF;
		str[1] = (c>>8) & 0xFF;
		str[2] = (c>>16) & 0xFF;
		str[3] = c>>24;
		str[4] = '\0';
	}
}

LFCore_API void LFUINTToString(const unsigned int v, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%d", v);
}

LFCore_API void LFINT64ToString(const __int64 v, wchar_t* str, size_t cCount)
{
	if (v>=0)
	{
		StrFormatByteSize(v, str, (unsigned int)cCount);
	}
	else
	{
		str[0] = L'\0';
	}
}

LFCore_API void LFFractionToString(const LFFraction frac, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%u/%u", frac.Num, frac.Denum);
}

LFCore_API void LFDoubleToString(const double d, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%.2f", d);
}

LFCore_API void LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude, bool FillZero)
{
	wchar_t Hemisphere[2];
	if (IsLatitude)
	{
		Hemisphere[0] = 'N';
		Hemisphere[1] = 'S';
	}
	else
	{
		Hemisphere[0] = 'W';
		Hemisphere[1] = 'E';
	}

	swprintf(str, cCount, FillZero ? L"%3u°%2u\'%2u\"%c" : L"%u°%u\'%u\"%c",
		(unsigned int)(fabs(c)+ROUNDOFF),
		(unsigned int)GetMinutes(c),
		(unsigned int)(GetSeconds(c)+0.5),
		Hemisphere[c>0]);

	if (FillZero)
		while (wchar_t* p=wcschr(str, L' '))
			*p = L'0';
}

LFCore_API void LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount, bool FillZero)
{
	if ((c.Latitude==0) && (c.Longitude==0))
	{
		wcscpy_s(str, cCount, L"");
	}
	else
	{
		wchar_t tmpStr[256];
		LFGeoCoordinateToString(c.Longitude, tmpStr, 256, false, FillZero);

		LFGeoCoordinateToString(c.Latitude, str, cCount, true, FillZero);
		wcscat_s(str, cCount, L", ");
		wcscat_s(str, cCount, tmpStr);
	}
}

LFCore_API void LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int mask)
{
	*str = '\0';

	if ((t.dwHighDateTime) || (t.dwLowDateTime))
	{
		SYSTEMTIME stUTC;
		SYSTEMTIME stLocal;
		FileTimeToSystemTime(&t, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

		if (mask & 1)
		{
			int cDate = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal, NULL, NULL, 0);
			if (cDate>256)
				cDate = 256;
			GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLocal, NULL, str, cDate);
		}

		if (mask==3)
			wcscat_s(str, cCount, L", ");

		if (mask & 2)
		{
			wchar_t tmpStr[256];
			int cTime = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &stLocal, NULL, NULL, 0);
			if (cTime>256)
				cTime = 256;
			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &stLocal, NULL, tmpStr, cTime);
			wcscat_s(str, cCount, tmpStr);
		}
	}
}

LFCore_API void LFDurationToString(unsigned int d, wchar_t* str, size_t cCount)
{
	d = (d+500)/1000;
	swprintf(str, cCount, L"%02d:%02d:%02d", d/3600, (d/60)%60, d%60);
}

LFCore_API void LFBitrateToString(const unsigned int r, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%d kBit/s", (r+500)/1000);
}

LFCore_API void LFMegapixelToString(const double d, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%.1f Megapixel", d);
}

void ToString(void* value, unsigned int type, wchar_t* str, size_t cCount)
{
	assert(type<LFTypeCount);
	assert(str);

	if (value)
		switch (type)
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			wcscpy_s(str, cCount, (wchar_t*)value);
			return;
		case LFTypeAnsiString:
			MultiByteToWideChar(CP_ACP, 0, (char*)value, -1, str, (int)cCount);
			return;
		case LFTypeFourCC:
			LFFourCCToString(*((unsigned int*)value), str, cCount);
			return;
		case LFTypeRating:
			assert(*((unsigned char*)value)<=LFMaxRating);
			wcscpy_s(str, cCount, &RatingStrings[5-*((unsigned char*)value)/2]);
			return;
		case LFTypeUINT:
			LFUINTToString(*((unsigned int*)value), str, cCount);
			return;
		case LFTypeINT64:
			LFINT64ToString(*((__int64*)value), str, cCount);
			return;
		case LFTypeFraction:
			LFFractionToString(*((LFFraction*)value), str, cCount);
			return;
		case LFTypeDouble:
			LFDoubleToString(*((double*)value), str, cCount);
			return;
		case LFTypeFlags:
			wchar_t FlagString[6];
			FlagString[0] = (*((unsigned int*)value) & LFFlagLink) ? 'L' : '-';
			FlagString[1] = (*((unsigned int*)value) & LFFlagNew) ? 'N' : '-';
			FlagString[2] = (*((unsigned int*)value) & LFFlagTrash) ? 'T' : '-';
			FlagString[3] = (*((unsigned int*)value) & LFFlagMissing) ? 'M' : '-';
			FlagString[4] = (*((unsigned int*)value) & LFFlagArchive) ? 'A' : '-';
			FlagString[5] = '\0';
			wcscpy_s(str, cCount, FlagString);
			return;
		case LFTypeGeoCoordinates:
			LFGeoCoordinatesToString(*((LFGeoCoordinates*)value), str, cCount, false);
			return;
		case LFTypeTime:
			LFTimeToString(*((FILETIME*)value), str, cCount, 3);
			return;
		case LFTypeDuration:
			LFDurationToString(*((unsigned int*)value), str, cCount);
			return;
		case LFTypeBitrate:
			LFBitrateToString(*((unsigned int*)value), str, cCount);
			return;
		case LFTypeMegapixel:
			LFMegapixelToString(*((double*)value), str, cCount);
			return;
		default:
			assert(false);
		}

	wcscpy_s(str, cCount, L"");
}

LFCore_API void LFAttributeToString(LFItemDescriptor* i, unsigned int attr, wchar_t* str, size_t cCount)
{
	assert(i);
	assert(attr<LFAttributeCount);
	assert(AttrTypes[attr]<LFTypeCount);

	if ((i->AggregateCount==0) && (attr==LFAttrFileCount))
	{
		str[0] = L'\0';
	}
	else
	{
		ToString(i->AttributeValues[attr], AttrTypes[attr], str, cCount);
	}
}


bool IsNullValue(unsigned int attr, void* v)
{
	if (!v)
		return true;

	assert(AttrTypes[attr]<LFTypeCount);

	switch (AttrTypes[attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (*(wchar_t*)v==L'\0');
	case LFTypeAnsiString:
		return (*(char*)v=='\0');
	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeFlags:
	case LFTypeDuration:
	case LFTypeBitrate:
		return (*(unsigned int*)v)==0;
	case LFTypeRating:
		return (*(unsigned char*)v)==0;
	case LFTypeINT64:
	case LFTypeTime:
		return (*(__int64*)v)==0;
	case LFTypeFraction:
		return (((LFFraction*)v)->Num==0) || (((LFFraction*)v)->Denum==0);
	case LFTypeDouble:
	case LFTypeMegapixel:
		return (*(double*)v)==-1;
	case LFTypeGeoCoordinates:
		return (((LFGeoCoordinates*)v)->Latitude==0) && (((LFGeoCoordinates*)v)->Longitude==0);
	}

	return false;
}

bool GetNextTag(wchar_t** tagarray, wchar_t* tag, size_t cCount)
{
	wchar_t* start = NULL;
	bool quotation = false;

	while (**tagarray!=L'\0')
	{
		switch (**tagarray)
		{
		case L' ':
		case L',':
		case L':':
		case L';':
		case L'|':
			if ((start) && (!quotation))
			{
				wcsncpy_s(tag, cCount, start, ((*tagarray)++)-start);
				return true;
			}
			break;
		case L'"':
			if (quotation)
			{
				quotation = false;
				if (start)
				{
					wcsncpy_s(tag, cCount, start, ((*tagarray)++)-start);
					return true;
				}
			}
			else
			{
				quotation = !start;
			}
			break;
		default:
			if (!start)
				start = *tagarray;
		}

		(*tagarray)++;
	}

	if (start)
	{
		wcscpy_s(tag, cCount, start);
		return true;
	}

	return false;
}


// LFVariantData
//

LFCore_API void LFVariantDataToString(LFVariantData* v, wchar_t* str, size_t cCount)
{
	assert(v);

	if (v->IsNull)
	{
		wcscpy_s(str, cCount, L"");
	}
	else
	{
		assert(v->Type<LFTypeCount);
		ToString(&v->Value, v->Type, str, cCount);
	}
}

LFCore_API void LFVariantDataFromString(LFVariantData* v, wchar_t* str)
{
	assert(v);

	LFGetNullVariantData(v);

	if ((str) && (v->Attr<LFAttributeCount))
	{
		size_t sz = wcslen(str);

		wchar_t tmpBuf[256];
		wchar_t* dst = &tmpBuf[0];

		unsigned int LatDeg;
		unsigned int LatMin;
		unsigned int LatSec;
		wchar_t LatCh;
		unsigned int LonDeg;
		unsigned int LonMin;
		unsigned int LonSec;
		wchar_t LonCh;

		unsigned int Date1;
		wchar_t DateCh1;
		unsigned int Date2;
		wchar_t DateCh2;
		unsigned int Date3;
		SYSTEMTIME stLocal;
		SYSTEMTIME stUTC;
		bool YearFirst;

		unsigned int Hour;
		unsigned int Min;
		unsigned int Sec;

		switch (v->Type)
		{
		case LFTypeUnicodeString:
			v->IsNull = false;
			wcscpy_s(v->UnicodeString, 256, str);
			break;
		case LFTypeUnicodeArray:
			v->IsNull = false;
			wcscpy_s(v->UnicodeArray, 256, str);
			LFSanitizeUnicodeArray(v->UnicodeArray, 256);
			break;
		case LFTypeAnsiString:
			v->IsNull = false;
			WideCharToMultiByte(CP_ACP, 0, str, -1, v->AnsiString, 256, NULL, NULL);
			break;
		case LFTypeFourCC:
			if (sz>=4)
			{
				v->IsNull = false;
				v->FourCC = (str[0]>0xFF ? L'_' : str[0]) |
					((str[1]>0xFF ? L'_' : str[1])<<8) |
					((str[2]>0xFF ? L'_' : str[2])<<16) |
					((str[3]>0xFF ? L'_' : str[3])<<24);
			}
			break;
		case LFTypeRating:
			if ((str[0]>=L'0') && (str[0]<=L'5'))
				if ((sz==1) || (sz==L' '))
				{
					v->IsNull = false;
					v->Rating = (unsigned char)((str[0]-L'0')*2);
					return;
				}
			if ((sz>=1) && (sz<=5))
			{
				bool same = true;
				for (unsigned int a=1; a<sz; a++)
					same &= (str[a]==str[a-1]);
				if (same)
				{
					v->IsNull = false;
					v->Rating = (unsigned char)(sz*2);
					return;
				}
			}
			break;
		case LFTypeUINT:
			if (swscanf_s(str, L"%d", &v->UINT)==1)
				v->IsNull = false;
			break;
		case LFTypeINT64:
			for (wchar_t* src = &str[0]; src<&tmpBuf[255]; src++)
			{
				switch (*src)
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
					*(dst++) = *src;
				case L'.':
				case L',':
					break;
				default:
					*dst = L'\0';
					break;
				}
			}
			if (swscanf_s(tmpBuf, L"%d", &v->INT64)==1)
				v->IsNull = false;
			break;
		case LFTypeFraction:
			if (swscanf_s(str, L"%u/%u", &v->Fraction.Num, &v->Fraction.Denum)==2)
				v->IsNull = false;
			break;
		case LFTypeDouble:
		case LFTypeMegapixel:
			if (swscanf_s(str, L"%f", &v->Double)==1)
				v->IsNull = false;
			break;
		case LFTypeGeoCoordinates:
			if (swscanf_s(str, L"%u°%u\'%u\"%c, %u°%u\'%u\"%c", &LatDeg, &LatMin, &LatSec, &LatCh, 1, &LonDeg, &LonMin, &LonSec, &LonCh, 1)==8)
				if (((LatCh==L'N') || (LatCh==L'S')) && ((LonCh==L'W') || (LonCh==L'E')))
				{
					v->GeoCoordinates.Latitude = LatDeg+(LatMin/60.0)+(LatSec/3600.0);
					if (LatCh==L'N')
						v->GeoCoordinates.Latitude -= v->GeoCoordinates.Latitude;
					v->GeoCoordinates.Longitude = LonDeg+(LonMin/60.0)+(LonSec/3600.0);
					if (LatCh==L'W')
						v->GeoCoordinates.Longitude -= v->GeoCoordinates.Longitude;
					v->IsNull = false;
				}
			break;
		case LFTypeTime:
			ZeroMemory(&stLocal, sizeof(stLocal));
			switch (swscanf_s(str, L"%u%c%u%c%u", &Date1, &DateCh1, 1, &Date2, &DateCh2, 1, &Date3))
			{
			case 1:
			case 2:
				if ((Date1>1600) && (Date1<0x10000))
				{
					stLocal.wYear = (WORD)Date1;
					stLocal.wMonth = stLocal.wDay = 1;

					TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
					SystemTimeToFileTime(&stUTC, &v->Time);
					v->IsNull = !((v->Time.dwHighDateTime) || (v->Time.dwLowDateTime));
				}
				break;
			case 3:
			case 4:
				if ((Date1>=0) && (Date1<=12) && (Date2>1600) && (Date2<0x10000))
					std::swap(Date1, Date2);
				if ((Date1>1600) && (Date1<0x10000) && (Date2>=0) && (Date2<=12))
				{
					stLocal.wYear = (WORD)Date1;
					stLocal.wMonth = (WORD)Date2;
					stLocal.wDay = 1;

					TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
					SystemTimeToFileTime(&stUTC, &v->Time);
					v->IsNull = !((v->Time.dwHighDateTime) || (v->Time.dwLowDateTime));
				}
				break;
			case 5:
				if ((DateCh1==DateCh2) && (Date1>0) && (Date2>0) && (Date3>0))
				{
					YearFirst = (Date1>1600) && (Date1<0x10000);
					if (YearFirst)
					{
						if ((DateCh1=='/') || ((Date2>12) && (Date3<=12)))
							std::swap(Date2, Date3);
						if ((Date2<=12) && (Date3<=31))
						{
							stLocal.wYear = (WORD)Date1;
							stLocal.wMonth = (WORD)Date2;
							stLocal.wDay = (WORD)Date3;

							TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
							SystemTimeToFileTime(&stUTC, &v->Time);
							v->IsNull = !((v->Time.dwHighDateTime) || (v->Time.dwLowDateTime));
						}
					}
					else
						if ((Date3>1600) && (Date3<0x10000))
						{
							if ((DateCh1=='/') || ((Date1>12) && (Date2<=12)))
								std::swap(Date1, Date2);
							if ((Date1<=31) && (Date2<=12))
							{
								stLocal.wYear = (WORD)Date3;
								stLocal.wMonth = (WORD)Date2;
								stLocal.wDay = (WORD)Date1;

								TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
								SystemTimeToFileTime(&stUTC, &v->Time);
								v->IsNull = !((v->Time.dwHighDateTime) || (v->Time.dwLowDateTime));
							}
						}
				}
			}
			break;
		case LFTypeDuration:
			if (swscanf_s(str, L"%d:%d:%d", &Hour, &Min, &Sec)==3)
			{
				v->Duration = 1000*(Hour*3600+Min*60+Sec);
				v->IsNull = false;
			}
			break;
		case LFTypeBitrate:
			if (swscanf_s(str, L"%d", &v->Bitrate)==1)
			{
				v->Bitrate *= 1000;
				v->IsNull = false;
			}
			break;
		}
	}
}

LFCore_API void LFGetNullVariantData(LFVariantData* v)
{
	assert(v);

	v->Type = (v->Attr<LFAttributeCount) ? AttrTypes[v->Attr] : LFTypeUnicodeString;
	v->IsNull = true;

	switch (v->Type)
	{
	case LFTypeDouble:
		v->Double = 0;
		break;
	case LFTypeGeoCoordinates:
		v->GeoCoordinates.Latitude = v->GeoCoordinates.Longitude = 0;
		break;
	case LFTypeMegapixel:
		v->Double = -1;
		break;
	default:
		ZeroMemory(v->UnicodeString, 512);
	}
}

LFCore_API bool LFIsNullVariantData(LFVariantData* v)
{
	if (v->IsNull)
		return true;

	assert(v->Attr<LFAttributeCount);
	assert(v->Type==AttrTypes[v->Attr]);

	return IsNullValue(v->Attr, &v->Value);
}

LFCore_API bool LFIsVariantDataEqual(LFVariantData* v1, LFVariantData* v2)
{
	if (v1->IsNull!=v2->IsNull)
		return false;
	if (v1->Type!=v2->Type)
		return false;

	assert(v1->Attr<LFAttributeCount);
	assert(v2->Attr<LFAttributeCount);
	assert(v1->Type==AttrTypes[v1->Attr]);
	assert(v2->Type==AttrTypes[v2->Attr]);
	assert(v1->Type<LFTypeCount);
	assert(v2->Type<LFTypeCount);

	switch (v1->Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return wcscmp(v1->UnicodeString, v2->UnicodeString)==0;
	case LFTypeAnsiString:
		return strcmp(v1->AnsiString, v2->AnsiString)==0;
	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeFlags:
	case LFTypeDuration:
	case LFTypeBitrate:
		return v1->UINT==v2->UINT;
	case LFTypeRating:
		return v1->Rating==v2->Rating;
	case LFTypeINT64:
		return v1->INT64==v2->INT64;
	case LFTypeFraction:
		return (v1->Fraction.Num==v2->Fraction.Num) && (v1->Fraction.Denum==v2->Fraction.Denum);
	case LFTypeDouble:
	case LFTypeMegapixel:
		return (v1->Double==v2->Double);
	case LFTypeGeoCoordinates:
		return (v1->GeoCoordinates.Latitude==v2->GeoCoordinates.Latitude) && (v1->GeoCoordinates.Longitude==v2->GeoCoordinates.Longitude);
	case LFTypeTime:
		return memcmp(&v1->Time, &v2->Time, sizeof(FILETIME))==0;
	}

	return false;
}

LFCore_API bool LFIsEqualToVariantData(LFItemDescriptor* i, LFVariantData* v)
{
	assert(i);
	assert(v);

	if (i->AttributeValues[v->Attr])
	{
		if (v->IsNull)
			return false;

		assert(v->Attr<LFAttributeCount);
		assert(v->Type==AttrTypes[v->Attr]);
		assert(v->Type<LFTypeCount);

		switch (v->Type)
		{
		case LFTypeUnicodeString:
			return wcscmp((wchar_t*)i->AttributeValues[v->Attr], v->UnicodeString)==0;
		case LFTypeUnicodeArray:
			return wcscmp((wchar_t*)i->AttributeValues[v->Attr], v->UnicodeArray)==0;
		case LFTypeAnsiString:
			return strcmp((char*)i->AttributeValues[v->Attr], v->AnsiString)==0;
		case LFTypeFourCC:
		case LFTypeUINT:
		case LFTypeFlags:
		case LFTypeDuration:
		case LFTypeBitrate:
			return *(unsigned int*)i->AttributeValues[v->Attr]==v->UINT;
		case LFTypeRating:
			return *(unsigned char*)i->AttributeValues[v->Attr]==v->Rating;
		case LFTypeINT64:
			return *(__int64*)i->AttributeValues[v->Attr]==v->INT64;
		case LFTypeFraction:
			return memcmp(i->AttributeValues[v->Attr], &v->Fraction, sizeof(LFFraction))==0;
		case LFTypeDouble:
		case LFTypeMegapixel:
			return (*(double*)i->AttributeValues[v->Attr]==v->Double);
		case LFTypeGeoCoordinates:
			return memcmp(i->AttributeValues[v->Attr], &v->GeoCoordinates, sizeof(LFGeoCoordinates))==0;
		case LFTypeTime:
			return memcmp(i->AttributeValues[v->Attr], &v->Time, sizeof(FILETIME))==0;
		}

		return false;
	}
	else
	{
		return v->IsNull;
	}
}

LFCore_API void LFGetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v)
{
	assert(i);
	assert(v);

	if (i->AttributeValues[v->Attr])
	{
		assert(v->Attr<LFAttributeCount);
		assert(v->Type==AttrTypes[v->Attr]);
		assert(v->Type<LFTypeCount);

		size_t sz = GetAttributeSize(v->Attr, i->AttributeValues[v->Attr]);
		memcpy(&v->Value, i->AttributeValues[v->Attr], sz);
		v->IsNull = false;
	}
	else
	{
		LFGetNullVariantData(v);
	}
}

LFCore_API void LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v)
{
	assert(i);
	assert(v->Attr<LFAttributeCount);
	assert(!v->IsNull);

	// Special treatment for flags
	if (v->Attr==LFAttrFlags)
	{
		v->Flags.Mask &= LFFlagTrash;
		v->Flags.Flags &= v->Flags.Mask;

		if (i->AttributeValues[v->Attr])
		{
			unsigned int f = ((*(unsigned int*)i->AttributeValues[v->Attr]) & (~v->Flags.Mask)) | v->Flags.Flags;
			SetAttribute(i, v->Attr, &f);
			return;
		}
	}

	// Other attributes
	SetAttribute(i, v->Attr, &v->Value);
}

LFCore_API void LFSanitizeUnicodeArray(wchar_t* buf, size_t cCount)
{
	typedef stdext::hash_map<std::wstring, unsigned int> hashcount;
	hashcount tags;

	wchar_t tag[259];
	wchar_t* tagarray = buf;
	while (GetNextTag(&tagarray, tag, 256))
	{
		bool first = true;
		for (wchar_t* ptr = tag; *ptr; ptr++)
			switch (*ptr)
			{
			case L' ':
			case L',':
			case L':':
			case L';':
			case L'|':
				first = true;
				break;
			default:
				*ptr = first ? (wchar_t)toupper(*ptr) : (wchar_t)tolower(*ptr);
				first = false;
			}

		tags[tag] = 1;
	}

	buf[0] = L'\0';
	for (hashcount::iterator it=tags.begin(); it!=tags.end(); it++)
	{
		tag[0] = L'\0';
		if (buf[0]!=L'\0')
			wcscpy_s(tag, cCount, L" ");

		if (it->first.find_first_of(L" ,:;|")!=std::wstring::npos)
		{
			wcscat_s(tag, 259, L"\"");
			wcscat_s(tag, 259, it->first.c_str());
			wcscat_s(tag, 259, L"\"");
		}
		else
		{
			wcscat_s(tag, 256, it->first.c_str());
		}

		if (wcslen(buf)+wcslen(tag)<=255)
			wcscat_s(buf, cCount, tag);
	}
}
