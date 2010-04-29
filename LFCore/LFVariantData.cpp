#include "StdAfx.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include <assert.h>
#include <wchar.h>


extern unsigned char AttrTypes[];


LFCore_API void LFGetNullVariantData(LFVariantData* v)
{
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

	assert(v1->Attr<LFAttributeCount);
	assert(v2->Attr<LFAttributeCount);
	assert(v1->Type==AttrTypes[v1->Attr]);
	assert(v2->Type==AttrTypes[v2->Attr]);
	assert(v1->Type<LFTypeCount);
	assert(v2->Type<LFTypeCount);

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

LFCore_API bool LFIsEqualToVariantData(LFItemDescriptor* i, LFVariantData* v)
{
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
		case LFTypeAnsiString:
			return strcmp((char*)i->AttributeValues[v->Attr], v->AnsiString)==0;
		case LFTypeFourCC:
		case LFTypeUINT:
		case LFTypeFlags:
		case LFTypeDuration:
			return *(unsigned int*)i->AttributeValues[v->Attr]==v->UINT;
		case LFTypeRating:
			return *(unsigned char*)i->AttributeValues[v->Attr]==v->Rating;
		case LFTypeINT64:
			return *(__int64*)i->AttributeValues[v->Attr]==v->INT64;
		case LFTypeFraction:
			return memcmp(i->AttributeValues[v->Attr], &v->Fraction, sizeof(LFFraction))==0;
		case LFTypeDouble:
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
	if (i->AttributeValues[v->Attr])
	{
		assert(v->Attr<LFAttributeCount);
		assert(v->Type==AttrTypes[v->Attr]);
		assert(v->Type<LFTypeCount);

		size_t sz = GetAttributeSize(v->Attr, i->AttributeValues[v->Attr]);
		memcpy_s(&v->Value, sz, i->AttributeValues[v->Attr], sz);
		v->IsNull = false;
	}
	else
	{
		LFGetNullVariantData(v);
	}
}

LFCore_API void LFSetAttributeVariantData(LFItemDescriptor* i, LFVariantData* v, wchar_t* ustr)
{
	assert(v->Attr<LFAttributeCount);
	assert(!v->IsNull);

	SetAttribute(i, v->Attr, &v->Value, true, ustr);
}
