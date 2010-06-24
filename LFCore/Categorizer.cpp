#include "StdAfx.h"
#include "Categorizer.h"
#include "LFCore.h"
#include <assert.h>


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

LFItemDescriptor* CCategorizer::GetFolder(LFItemDescriptor* i)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;
	strcpy_s(folder->StoreID, LFKeySize, i->StoreID);

	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		LFAttributeToString(i, attr, Name, 256);
		SetAttribute(folder, attr, i->AttributeValues[attr]);
		SetAttribute(folder, LFAttrFileName, Name);
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

LFItemDescriptor* DateCategorizer::GetFolder(LFItemDescriptor* i)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;
	strcpy_s(folder->StoreID, LFKeySize, i->StoreID);

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

LFItemDescriptor* RatingCategorizer::GetFolder(LFItemDescriptor* i)
{
	LFItemDescriptor* folder = LFAllocItemDescriptor();
	folder->Type = LFTypeVirtual;
	strcpy_s(folder->StoreID, LFKeySize, i->StoreID);

	if (i->AttributeValues[attr])
	{
		wchar_t Name[256];
		wcscpy_s(Name,256,L"XXX");
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

LFItemDescriptor* IATACategorizer::GetFolder(LFItemDescriptor* i)
{
	if (i->AttributeValues[attr])
	{
		LFAirport* airport;
		if (LFIATAGetAirportByCode((char*)i->AttributeValues[attr], &airport))
		{
			LFItemDescriptor* folder = LFIATACreateFolderForAirport(airport);
			strcpy_s(folder->StoreID, LFKeySize, i->StoreID);
			return folder;
		}
	}

	return CCategorizer::GetFolder(i);
}
