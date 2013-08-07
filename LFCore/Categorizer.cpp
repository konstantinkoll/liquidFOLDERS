
#include "stdafx.h"
#include "Categorizer.h"
#include "LFCore.h"
#include "IATA.h"
#include "Query.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern unsigned char AttrTypes[];


// CCategorizer
//

CCategorizer::CCategorizer(unsigned int _attr)
{
	attr = _attr;
}

bool CCategorizer::IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	if ((!i1->AttributeValues[attr]) || (!i2->AttributeValues[attr]))
		return i1->AttributeValues[attr]==i2->AttributeValues[attr];
	if (((i1->Type & LFTypeMask)==LFTypeVirtual) || ((i2->Type & LFTypeMask)==LFTypeVirtual))
		return false;

	return Compare(i1, i2);
}

LFItemDescriptor* CCategorizer::GetFolder(LFItemDescriptor* i, LFFilter* f)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;

	if (i->AttributeValues[attr])
	{
		folder->NextFilter = LFAllocFilter(f);
		folder->NextFilter->Options.IsSubfolder = true;
		folder->NextFilter->Options.GroupAttribute = attr;

		LFFilterCondition* c = GetCondition(i);
		c->Next = folder->NextFilter->ConditionList;
		folder->NextFilter->ConditionList = c;
	}

	CustomizeFolder(folder, i);

	if (folder->NextFilter)
		wcscpy_s(folder->NextFilter->OriginalName, 256, folder->CoreAttributes.FileName);

	return folder;
}

bool CCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	wchar_t First[256];
	wchar_t Second[256];

	LFAttributeToString(i1, attr, First, 256);
	LFAttributeToString(i2, attr, Second, 256);

	return (wcscmp(First, Second)==0);
}

void CCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LFAttributeToString(i, attr, Name, 256);
		SetAttribute(folder, attr, i->AttributeValues[attr]);
		SetAttribute(folder, LFAttrFileName, Name);
	}
}

LFFilterCondition* CCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = attr;
	c->AttrData.Type = AttrTypes[attr];
	LFGetAttributeVariantData(i, &c->AttrData);

	return c;
}


// DateCategorizer
//

DateCategorizer::DateCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool DateCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeTime);

	SYSTEMTIME stUTC1;
	SYSTEMTIME stUTC2;
	SYSTEMTIME stLocal1;
	SYSTEMTIME stLocal2;
	FileTimeToSystemTime((FILETIME*)i1->AttributeValues[attr], &stUTC1);
	FileTimeToSystemTime((FILETIME*)i2->AttributeValues[attr], &stUTC2);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC1, &stLocal1);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC2, &stLocal2);

	return (stLocal1.wDay==stLocal2.wDay) && (stLocal1.wMonth==stLocal2.wMonth) && (stLocal1.wYear==stLocal2.wYear);
}

void DateCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		FILETIME ft = *((FILETIME*)i->AttributeValues[attr]);
		LFTimeToString(ft, Name, 256, 1);

		SYSTEMTIME stUTC;
		SYSTEMTIME stLocal;
		FileTimeToSystemTime(&ft, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

		stLocal.wHour = stLocal.wMinute = stLocal.wSecond = stLocal.wMilliseconds = 0;

		TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
		SystemTimeToFileTime(&stUTC, &ft);
		SetAttribute(folder, attr, &ft);

		SetAttribute(folder, LFAttrFileName, Name);
	}
}

LFFilterCondition* DateCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = attr;
	c->AttrData.Type = AttrTypes[attr];
	c->AttrData.IsNull = false;

	FILETIME ft = *((FILETIME*)i->AttributeValues[attr]);

	SYSTEMTIME stUTC;
	SYSTEMTIME stLocal;
	FileTimeToSystemTime(&ft, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	stLocal.wHour = stLocal.wMinute = stLocal.wSecond = stLocal.wMilliseconds = 0;

	TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
	SystemTimeToFileTime(&stUTC, &c->AttrData.Time);

	return c;
}


// RatingCategorizer
//

RatingCategorizer::RatingCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool RatingCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeRating);

	return GetRatingCategory(*((unsigned char*)i1->AttributeValues[attr]))==GetRatingCategory(*((unsigned char*)i2->AttributeValues[attr]));
}

void RatingCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LoadString(LFCoreModuleHandle, ((attr==LFAttrRating) ? IDS_Rating1 : IDS_Priority1)+GetRatingCategory(*((unsigned char*)i->AttributeValues[attr]))-1, Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);

		unsigned char rating = GetRatingCategory(*((unsigned char*)i->AttributeValues[attr]))*2;
		SetAttribute(folder, attr, &rating);
	}
}


// UnicodeCategorizer
//

UnicodeCategorizer::UnicodeCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool UnicodeCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeUnicodeString);

	return attr ? wcscmp((wchar_t*)i1->AttributeValues[attr], (wchar_t*)i2->AttributeValues[attr])==0 :
		_wcsicmp((wchar_t*)i1->AttributeValues[attr], (wchar_t*)i2->AttributeValues[attr])==0;
}

LFFilterCondition* UnicodeCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = attr ? LFFilterCompareSubfolder : LFFilterCompareIsEqual;

	c->AttrData.Attr = attr;
	c->AttrData.Type = AttrTypes[attr];
	LFGetAttributeVariantData(i, &c->AttrData);

	return c;
}


// AnsiCategorizer
//

AnsiCategorizer::AnsiCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool AnsiCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeAnsiString);

	return strcmp((char*)i1->AttributeValues[attr], (char*)i2->AttributeValues[attr])==0;
}


// UINTCategorizer
//

UINTCategorizer::UINTCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool UINTCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeUINT);

	return *((unsigned int*)i1->AttributeValues[attr])==*((unsigned int*)i2->AttributeValues[attr]);
}


// IATACategorizer
//

IATACategorizer::IATACategorizer(unsigned int _attr)
	: AnsiCategorizer(_attr)
{
}

void IATACategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		LFAirport* airport;
		if (LFIATAGetAirportByCode((char*)i->AttributeValues[attr], &airport))
		{
			CustomizeFolderForAirport(folder, airport);
			return;
		}
	}

	CCategorizer::CustomizeFolder(folder, i);
}


// CoordCategorizer
//

CoordCategorizer::CoordCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool CoordCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeGeoCoordinates);

	return memcmp(i1->AttributeValues[attr], i2->AttributeValues[attr], sizeof(LFGeoCoordinates))==0;
}


// SizeCategorizer
//

SizeCategorizer::SizeCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool SizeCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeINT64);

	return GetSizeCategory(*((__int64*)i1->AttributeValues[attr]))==GetSizeCategory(*((__int64*)i2->AttributeValues[attr]));
}

void SizeCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LoadString(LFCoreModuleHandle, IDS_Size1+GetSizeCategory(*((__int64*)i->AttributeValues[attr])), Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, attr, i->AttributeValues[attr]);
	}
}


// DurationCategorizer
//

DurationCategorizer::DurationCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool DurationCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeINT64);

	return GetDurationCategory(*((unsigned int*)i1->AttributeValues[attr]))==GetDurationCategory(*((unsigned int*)i2->AttributeValues[attr]));
}

void DurationCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LoadString(LFCoreModuleHandle, IDS_Duration1+GetDurationCategory(*((unsigned int*)i->AttributeValues[attr])), Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, attr, i->AttributeValues[attr]);
	}
}


// NameCategorizer
//

NameCategorizer::NameCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool NameCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeUnicodeString);

	wchar_t Prefix1[256];
	wchar_t Prefix2[256];

	bool res1 = GetNamePrefix((wchar_t*)i1->AttributeValues[attr], Prefix1);
	bool res2 = GetNamePrefix((wchar_t*)i2->AttributeValues[attr], Prefix2);

	return (res1 & res2) ? wcscmp(Prefix1, Prefix2)==0 : false;
}

void NameCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		if (!GetNamePrefix((wchar_t*)i->AttributeValues[attr], &Name[0]))
			wcscpy_s(Name, 256, i->CoreAttributes.FileName);

		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, attr, Name);
	}
}

LFFilterCondition* NameCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = attr;
	c->AttrData.Type = AttrTypes[attr];
	c->AttrData.IsNull = false;

	if (!GetNamePrefix((wchar_t*)i->AttributeValues[attr], c->AttrData.UnicodeString))
		LFGetAttributeVariantData(i, &c->AttrData);

	return c;
}


// DurationBitrateCategorizer
//

DurationBitrateCategorizer::DurationBitrateCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool DurationBitrateCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert((AttrTypes[attr]==LFTypeDuration) || (AttrTypes[attr]==LFTypeBitrate));

	return ((*((unsigned int*)i1->AttributeValues[attr])+500)/1000)==((*((unsigned int*)i2->AttributeValues[attr])+500)/1000);
}


// MegapixelCategorizer
//

MegapixelCategorizer::MegapixelCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool MegapixelCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeMegapixel);

	return (unsigned int)*((double*)i1->AttributeValues[attr])==(unsigned int)*((double*)i2->AttributeValues[attr]);
}

void MegapixelCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		unsigned int dimension = (unsigned int)*((double*)i->AttributeValues[attr]);
		double d = dimension;

		wchar_t Name[256];
		swprintf_s(Name, 256, L"%u Megapixel", dimension);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, attr, &d);
	}
}

LFFilterCondition* MegapixelCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;
	c->AttrData.Attr = attr;
	c->AttrData.Type = AttrTypes[attr];
	c->AttrData.IsNull = false;
	c->AttrData.Megapixel = (unsigned int)*((double*)i->AttributeValues[attr]);

	return c;
}


// URLCategorizer
//

URLCategorizer::URLCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool URLCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeAnsiString);

	char Server1[256];
	char Server2[256];

	GetServer((char*)i1->AttributeValues[attr], Server1);
	GetServer((char*)i2->AttributeValues[attr], Server2);

	return strcmp(Server1, Server2)==0;
}

void URLCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		char Server[256];
		GetServer((char*)i->AttributeValues[attr], Server);

		wchar_t Name[256];
		MultiByteToWideChar(CP_ACP, 0, Server, -1, Name, 256);

		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, attr, Server);
	}
}

LFFilterCondition* URLCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = attr;
	c->AttrData.Type = AttrTypes[attr];
	c->AttrData.IsNull = false;

	GetServer((char*)i->AttributeValues[attr], c->AttrData.AnsiString);

	return c;
}
