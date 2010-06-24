#include "StdAfx.h"
#include "Categorizer.h"
#include "LFCore.h"
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
		wchar_t Name[256];
		LFAttributeToString(i, attr, Name, 256);
		SetAttribute(folder, attr, i->AttributeValues[attr]);
		SetAttribute(folder, LFAttrFileName, Name);

		folder->NextFilter = LFAllocFilter(f);
		folder->NextFilter->Options.IsSubfolder = true;

		LFFilterCondition* c = new LFFilterCondition;
		c->AttrData.Attr = attr;
		c->AttrData.Type = AttrTypes[attr];
		LFGetAttributeVariantData(i, &c->AttrData);
		c->Compare = LFFilterCompareIsEqual;

		c->Next = folder->NextFilter->ConditionList;
		folder->NextFilter->ConditionList = c;
	}

	return folder;
}

bool CCategorizer::Compare(LFItemDescriptor* /*i1*/, LFItemDescriptor* /*i2*/)
{
	return false;
}


// DateCategorizer
//

DateCategorizer::DateCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

LFItemDescriptor* DateCategorizer::GetFolder(LFItemDescriptor* i, LFFilter* f)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;
	strcpy_s(folder->StoreID, LFKeySize, f->StoreID);

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

	return folder;
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


// RatingCategorizer
//

RatingCategorizer::RatingCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

LFItemDescriptor* RatingCategorizer::GetFolder(LFItemDescriptor* i, LFFilter* f)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;
	strcpy_s(folder->StoreID, LFKeySize, f->StoreID);

	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LoadString(LFCoreModuleHandle, ((attr==LFAttrRating) ? IDS_Rating1 : IDS_Priority1)+*((unsigned char*)i->AttributeValues[attr])/2-1, Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);

		unsigned char rating = *((unsigned char*)i->AttributeValues[attr]) & 0xFE;
		SetAttribute(folder, attr, &rating);
	}

	return folder;
}

bool RatingCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeRating);

	return (*((unsigned char*)i1->AttributeValues[attr])/2)==(*((unsigned char*)i2->AttributeValues[attr])/2);
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

LFItemDescriptor* IATACategorizer::GetFolder(LFItemDescriptor* i, LFFilter* f)
{
	if (i->AttributeValues[attr])
	{
		LFAirport* airport;
		if (LFIATAGetAirportByCode((char*)i->AttributeValues[attr], &airport))
		{
			LFItemDescriptor* folder = LFIATACreateFolderForAirport(airport);
			strcpy_s(folder->StoreID, LFKeySize, f->StoreID);
			return folder;
		}
	}

	return CCategorizer::GetFolder(i, f);
}


// SizeCategorizer
//

SizeCategorizer::SizeCategorizer(unsigned int _attr)
	: CCategorizer(_attr)
{
}

LFItemDescriptor* SizeCategorizer::GetFolder(LFItemDescriptor* i, LFFilter* f)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;
	strcpy_s(folder->StoreID, LFKeySize, f->StoreID);

	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LoadString(LFCoreModuleHandle, IDS_Size1+GetCategory(*((__int64*)i->AttributeValues[attr])), Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, attr, i->AttributeValues[attr]);
	}

	return folder;
}

bool SizeCategorizer::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[attr]==LFTypeINT64);

	return GetCategory(*((__int64*)i1->AttributeValues[attr]))==GetCategory(*((__int64*)i2->AttributeValues[attr]));
}

unsigned int SizeCategorizer::GetCategory(const __int64 sz)
{
	if (sz<32*1024)
		return 0;
	if (sz<128*1024)
		return 1;
	if (sz<1024*1024)
		return 2;
	if (sz<16384*1024)
		return 3;
	if (sz<131072*1024)
		return 4;
	return 5;
}
