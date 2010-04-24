#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFVariantData.h"
#include <cmath>
#include <shlwapi.h>
#include <stdio.h>
#include <wchar.h>


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_variant")

wchar_t RatingString[6] = L"*****";
wchar_t FlagStrings[8][4] = { L"---", L"--T", L"-N-", L"-NT", L"L--", L"L-T", L"LN-", L"LNT" };

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_variant,RWS")


// Conversion ToString
//

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
	StrFormatByteSizeW(v, str, (unsigned int)cCount);
}

LFCore_API void LFFractionToString(const LFFraction frac, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%u/%u", frac.Num, frac.Denum);
}

LFCore_API void LFDoubleToString(const double d, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%f", d);
}

LFCore_API void LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude)
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

	swprintf(str, cCount, L"%u°%u\'%u\"%c",
		(unsigned int)(fabs(c)+ROUNDOFF),
		(unsigned int)GetMinutes(c),
		(unsigned int)(GetSeconds(c)+0.5),
		Hemisphere[c>0]);
}

LFCore_API void LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount)
{
	if ((c.Latitude==0) && (c.Longitude==0))
	{
		wcscpy_s(str, cCount, L"");
	}
	else
	{
		wchar_t tmpStr[256];
		LFGeoCoordinateToString(c.Longitude, tmpStr, 256, false);

		LFGeoCoordinateToString(c.Latitude, str, cCount, true);
		wcscat_s(str, cCount, L", ");
		wcscat_s(str, cCount, tmpStr);
	}
}

LFCore_API void LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int mask)
{
	*str = '\0';

	if ((t.dwHighDateTime) || (t.dwLowDateTime))
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&t, &st);

		if (mask & 1)
		{
			int cDate = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, NULL, 0);
			if (cDate>256)
				cDate = 256;
			GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, str, cDate);
		}

		if (mask==3)
			wcscat_s(str, cCount, L", ");

		if (mask & 2)
		{
			wchar_t tmpStr[256];
			int cTime = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &st, NULL, NULL, 0);
			if (cTime>256)
				cTime = 256;
			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &st, NULL, tmpStr, cTime);
			wcscat_s(str, cCount, tmpStr);
		}
	}
}

LFCore_API void LFDurationToString(unsigned int d, wchar_t* str, size_t cCount)
{
	d = (d+999)/1000;
	swprintf(str, cCount, L"%02d:%02d:%02d", d/3600, (d/60)%60, d%60);
}


// Attribute handling
//

void FreeAttribute(LFItemDescriptor* f, unsigned int attr)
{
	if ((attr!=LFAttrRating) && (attr!=LFAttrFlags))
	{
		// Repräsentation als Unicode-String nur dann freigeben, wenn ungleich dem eigentlich Attributwert
		if (f->AttributeStrings[attr])
			if (f->AttributeStrings[attr]!=f->AttributeValues[attr])
			{
				free(f->AttributeStrings[attr]);
				f->AttributeStrings[attr] = NULL;
			}

		// Attributwert nur dann freigeben wenn er nicht statischer Teil des LFItemDescriptor ist
		if (attr>LFLastLocalAttribute)
			if (f->AttributeValues[attr])
			{
				free(f->AttributeValues[attr]);
				f->AttributeValues[attr] = NULL;
			}
	}
}

inline void StoreAttributeString(LFItemDescriptor*f, unsigned int attr, const wchar_t* ustr)
{
	size_t sz = wcslen(ustr)+1;
	if (sz>1)
	{
		f->AttributeStrings[attr] = (wchar_t*)malloc(sz*sizeof(wchar_t));
		wcscpy_s(f->AttributeStrings[attr], sz, ustr);
	}
}

inline void StoreAttributeValue(LFItemDescriptor*f, unsigned int attr, size_t sz, const void* v, wchar_t* ustr=NULL)
{
	FreeAttribute(f, attr);

	if (attr>LFLastLocalAttribute)
		f->AttributeValues[attr] = malloc(sz);
	if (f->AttributeValues[attr])
		memcpy_s(f->AttributeValues[attr], sz, v, sz);

	if (ustr)
		StoreAttributeString(f, attr, ustr);
}

void SetAttributeUnicodeString(LFItemDescriptor* f, unsigned int attr, const wchar_t* str)
{
	size_t sz = wcslen(str)+1;
	StoreAttributeValue(f, attr, sz*sizeof(wchar_t), str);

	if (attr>LFLastLocalAttribute)
		f->AttributeStrings[attr] = (wchar_t*)f->AttributeValues[attr];
}

void SetAttributeAnsiString(LFItemDescriptor* f, unsigned int attr, const char* str, wchar_t* ustr)
{
	size_t sz = strlen(str)+1;
	StoreAttributeValue(f, attr, sz, str, ustr);

	if (!ustr)
	{
		f->AttributeStrings[attr] = (wchar_t*)malloc(sz*sizeof(wchar_t));
		if (f->AttributeStrings[attr])
			MultiByteToWideChar(CP_ACP, 0, str, (int)sz, (LPWSTR)f->AttributeStrings[attr], (int)sz);
	}
}

void SetAttributeFourCC(LFItemDescriptor* f, unsigned int attr, unsigned int c, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(unsigned int), &c, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[5];
		LFFourCCToString(c, tmpStr, 5);
		StoreAttributeString(f, attr, tmpStr);
	}
}

void SetAttributeRating(LFItemDescriptor* f, unsigned int attr, unsigned char r)
{
	if (r>LFMaxRating)
		r=0;

	StoreAttributeValue(f, attr, sizeof(unsigned char), &r);
	f->AttributeStrings[attr] = &RatingString[5-r/2];
}

void SetAttributeUINT(LFItemDescriptor* f, unsigned int attr, unsigned int v, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(unsigned int), &v, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[256];
		LFUINTToString(v, tmpStr, 256);
		StoreAttributeString(f, attr, tmpStr);
	}
}

void SetAttributeINT64(LFItemDescriptor* f, unsigned int attr, __int64 v, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(__int64), &v, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[256];
		LFINT64ToString(v, tmpStr, 256);
		StoreAttributeString(f, attr, tmpStr);
	}
}

void SetAttributeFraction(LFItemDescriptor* f, unsigned int attr, const LFFraction frac, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(LFFraction), &frac, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[256];
		LFFractionToString(frac, tmpStr, 256);
		StoreAttributeString(f, attr, tmpStr);
	}
}

void SetAttributeDouble(LFItemDescriptor* f, unsigned int attr, double d, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(double), &d, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[256];
		LFDoubleToString(d, tmpStr, 256);
		StoreAttributeString(f, attr, tmpStr);
	}
}

void SetAttributeFlags(LFItemDescriptor* f, unsigned int attr, unsigned int v)
{
	v &= (1<<(LFLastFlagBit+1))-1;

	StoreAttributeValue(f, attr, sizeof(unsigned int), &v);
	f->AttributeStrings[attr] = &FlagStrings[v][0];
}

void SetAttributeGeoCoordinates(LFItemDescriptor* f, unsigned int attr, const LFGeoCoordinates c, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(LFGeoCoordinates), &c, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[256];
		LFGeoCoordinatesToString(c, tmpStr, 256);
		StoreAttributeString(f, attr, tmpStr);
	}
}

void SetAttributeTime(LFItemDescriptor* f, unsigned int attr, const FILETIME t, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(FILETIME), &t, ustr);

	if (!ustr)
		if ((t.dwHighDateTime) || (t.dwLowDateTime))
		{
			wchar_t tmpStr[256];
			LFTimeToString(t, tmpStr, 256);
			StoreAttributeString(f, attr, tmpStr);
		}
		else
		{
			if (f->AttributeStrings[attr])
				*f->AttributeStrings[attr] = '\0';
		}
}

void SetAttributeDuration(LFItemDescriptor* f, unsigned int attr, unsigned int d, wchar_t* ustr)
{
	StoreAttributeValue(f, attr, sizeof(unsigned int), &d, ustr);

	if (!ustr)
	{
		wchar_t tmpStr[256];
		LFDurationToString(d, tmpStr, 256);
		StoreAttributeString(f, attr, tmpStr);
	}
}


// Variant Data handling
//

LFCore_API void LFGetNullVariantData(LFVariantData* v, unsigned char _Type)
{
	v->Type = _Type;
	v->IsNull = true;

	switch (_Type)
	{
	case LFTypeDouble:
		v->Double = 0;
		break;
	case LFTypeGeoCoordinates:
		v->GeoCoordinates.Latitude = v->GeoCoordinates.Longitude = 0;
		break;
	default:
		ZeroMemory(v->UnicodeString, 512);
	}
}

LFCore_API bool LFIsVariantDataEqual(LFVariantData* v1, LFVariantData* v2)
{
	if (v1->IsNull!=v2->IsNull)
		return false;
	if (v1->Type!=v2->Type)
		return false;

	switch (v1->Type)
	{
	case LFTypeUnicodeString:
		return wcscmp(v1->UnicodeString, v2->UnicodeString)==0;
	case LFTypeAnsiString:
		return strcmp(v1->AnsiString, v2->AnsiString)==0;
	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeFlags:
	case LFTypeDuration:
		return v1->UINT==v2->UINT;
	case LFTypeRating:
		return v1->Rating==v2->Rating;
	case LFTypeINT64:
		return v1->INT64==v2->INT64;
	case LFTypeFraction:
		return (v1->Fraction.Num==v2->Fraction.Num) && (v1->Fraction.Denum==v2->Fraction.Denum);
	case LFTypeDouble:
		return (v1->Double==v2->Double);
	case LFTypeGeoCoordinates:
		return (v1->GeoCoordinates.Latitude==v2->GeoCoordinates.Latitude) && (v1->GeoCoordinates.Longitude==v2->GeoCoordinates.Longitude);
	case LFTypeTime:
		return memcmp(&v1->Time, &v2->Time, sizeof(FILETIME))==0;
	}

	return false;
}

LFCore_API bool LFIsEqualToVariantData(LFItemDescriptor* f, LFVariantData* v)
{
	if (f->AttributeValues[v->Attr])
	{
		if (v->IsNull)
			return false;

		switch (v->Type)
		{
		case LFTypeUnicodeString:
			return wcscmp((wchar_t*)f->AttributeValues[v->Attr], v->UnicodeString)==0;
		case LFTypeAnsiString:
			return strcmp((char*)f->AttributeValues[v->Attr], v->AnsiString)==0;
		case LFTypeFourCC:
		case LFTypeUINT:
		case LFTypeFlags:
		case LFTypeDuration:
			return *(unsigned int*)f->AttributeValues[v->Attr]==v->UINT;
		case LFTypeRating:
			return *(unsigned char*)f->AttributeValues[v->Attr]==v->Rating;
		case LFTypeINT64:
			return *(__int64*)f->AttributeValues[v->Attr]==v->INT64;
		case LFTypeFraction:
			return memcmp(f->AttributeValues[v->Attr], &v->Fraction, sizeof(LFFraction))==0;
		case LFTypeDouble:
			return (*(double*)f->AttributeValues[v->Attr]==v->Double);
		case LFTypeGeoCoordinates:
			return memcmp(f->AttributeValues[v->Attr], &v->GeoCoordinates, sizeof(LFGeoCoordinates))==0;
		case LFTypeTime:
			return memcmp(f->AttributeValues[v->Attr], &v->Time, sizeof(FILETIME))==0;
		}

		return false;
	}
	else
	{
		return v->IsNull;
	}
}
