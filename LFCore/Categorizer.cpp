#include "StdAfx.h"
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
	strcpy_s(folder->StoreID, LFKeySize, f->StoreID);

	if (i->AttributeValues[attr])
	{
		folder->NextFilter = LFAllocFilter(f);
		folder->NextFilter->Options.IsSubfolder = true;

		LFFilterCondition* c = new LFFilterCondition;
		c->AttrData.Attr = attr;
		c->AttrData.Type = AttrTypes[attr];
		LFGetAttributeVariantData(i, &c->AttrData);
		c->Compare = LFFilterCompareSubfolder;

		c->Next = folder->NextFilter->ConditionList;
		folder->NextFilter->ConditionList = c;
	}

	CustomizeFolder(folder, i);
	
	if (folder->NextFilter)
		wcscpy_s(folder->NextFilter->Name, 256, folder->CoreAttributes.FileName);

	return folder;
}

bool CCategorizer::Compare(LFItemDescriptor* /*i1*/, LFItemDescriptor* /*i2*/)
{
	return false;
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


// DateCategorizer
//

DateCategorizer::DateCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

bool DateCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeTime);

	SYSTEMTIME st1;
	SYSTEMTIME st2;
	FileTimeToSystemTime((FILETIME*)i1->AttributeValues[attr], &st1);
	FileTimeToSystemTime((FILETIME*)i2->AttributeValues[attr], &st2);

	return (st1.wDay==st2.wDay) && (st1.wMonth==st2.wMonth) && (st1.wYear==st2.wYear);
}

void DateCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		FILETIME ft;
		SYSTEMTIME st;

		ft = *((FILETIME*)i->AttributeValues[attr]);
		LFTimeToString(ft, Name, 256, 1);

		FileTimeToSystemTime(&ft, &st);
		st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;
		SystemTimeToFileTime(&st, &ft);
		SetAttribute(folder, attr, &ft);

		SetAttribute(folder, LFAttrFileName, Name);
	}
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

	return wcscmp((wchar_t*)i1->AttributeValues[attr], (wchar_t*)i2->AttributeValues[attr])==0;
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

	return CCategorizer::CustomizeFolder(folder, i);
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
