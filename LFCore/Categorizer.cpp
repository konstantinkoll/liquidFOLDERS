
#include "stdafx.h"
#include "Categorizer.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Query.h"
//#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern const BYTE AttrTypes[];


// CCategorizer
//

CCategorizer::CCategorizer(UINT Attr)
{
	m_Attr = Attr;
}

BOOL CCategorizer::IsEqual(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	if ((!i1->AttributeValues[m_Attr]) || (!i2->AttributeValues[m_Attr]))
		return i1->AttributeValues[m_Attr]==i2->AttributeValues[m_Attr];

	return CompareItems(i1, i2);
}

LFItemDescriptor* CCategorizer::GetFolder(LFItemDescriptor* i, LFFilter* f)
{
	LFItemDescriptor* folder = AllocFolderDescriptor();

	if (i->AttributeValues[m_Attr])
	{
		folder->NextFilter = LFAllocFilter(f);
		folder->NextFilter->Options.IsSubfolder = TRUE;
		folder->NextFilter->Options.GroupAttribute = m_Attr;

		LFFilterCondition* c = GetCondition(i);
		c->Next = folder->NextFilter->ConditionList;
		folder->NextFilter->ConditionList = c;
	}

	CustomizeFolder(folder, i);

	if (folder->NextFilter)
		wcscpy_s(folder->NextFilter->OriginalName, 256, folder->CoreAttributes.FileName);

	return folder;
}

BOOL CCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	return CompareValues(AttrTypes[m_Attr], i1->AttributeValues[m_Attr], i2->AttributeValues[m_Attr], m_Attr>0)==0;
}

void CCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		WCHAR Name[256];
		LFAttributeToString(i, m_Attr, Name, 256);
		SetAttribute(folder, m_Attr, i->AttributeValues[m_Attr]);
		SetAttribute(folder, LFAttrFileName, Name);
	}
}

LFFilterCondition* CCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = m_Attr ? LFFilterCompareSubfolder : LFFilterCompareIsEqual;

	LFGetAttributeVariantDataEx(i, m_Attr, c->AttrData);

	return c;
}


// CDateCategorizer
//

CDateCategorizer::CDateCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL CDateCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeTime);

	SYSTEMTIME stUTC1;
	SYSTEMTIME stUTC2;
	SYSTEMTIME stLocal1;
	SYSTEMTIME stLocal2;
	FileTimeToSystemTime((FILETIME*)i1->AttributeValues[m_Attr], &stUTC1);
	FileTimeToSystemTime((FILETIME*)i2->AttributeValues[m_Attr], &stUTC2);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC1, &stLocal1);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC2, &stLocal2);

	return (stLocal1.wDay==stLocal2.wDay) && (stLocal1.wMonth==stLocal2.wMonth) && (stLocal1.wYear==stLocal2.wYear);
}

void CDateCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		WCHAR Name[256];
		FILETIME ft = *((FILETIME*)i->AttributeValues[m_Attr]);
		LFTimeToString(ft, Name, 256, FALSE);

		SYSTEMTIME stUTC;
		SYSTEMTIME stLocal;
		FileTimeToSystemTime(&ft, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

		stLocal.wHour = stLocal.wMinute = stLocal.wSecond = stLocal.wMilliseconds = 0;

		TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
		SystemTimeToFileTime(&stUTC, &ft);
		SetAttribute(folder, m_Attr, &ft);

		SetAttribute(folder, LFAttrFileName, Name);
	}
}

LFFilterCondition* CDateCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = m_Attr;
	c->AttrData.Type = AttrTypes[m_Attr];
	c->AttrData.IsNull = FALSE;

	FILETIME ft = *((FILETIME*)i->AttributeValues[m_Attr]);

	SYSTEMTIME stUTC;
	SYSTEMTIME stLocal;
	FileTimeToSystemTime(&ft, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	stLocal.wHour = stLocal.wMinute = stLocal.wSecond = stLocal.wMilliseconds = 0;

	TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
	SystemTimeToFileTime(&stUTC, &c->AttrData.Time);

	return c;
}


// CRatingCategorizer
//

CRatingCategorizer::CRatingCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL CRatingCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeRating);

	return GetRatingCategory(*((BYTE*)i1->AttributeValues[m_Attr]))==GetRatingCategory(*((BYTE*)i2->AttributeValues[m_Attr]));
}

void CRatingCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		WCHAR Name[256];
		LoadString(LFCoreModuleHandle, ((m_Attr==LFAttrRating) ? IDS_RATING1 : IDS_PRIORITY1)+GetRatingCategory(*((BYTE*)i->AttributeValues[m_Attr]))-1, Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);

		BYTE rating = GetRatingCategory(*((BYTE*)i->AttributeValues[m_Attr]))*2;
		SetAttribute(folder, m_Attr, &rating);
	}
}


// IATACategorizer
//

IATACategorizer::IATACategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

void IATACategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		LFAirport* airport;
		if (LFIATAGetAirportByCode((CHAR*)i->AttributeValues[m_Attr], &airport))
		{
			// Ortsname in ANSI
			CHAR tmpStr1[256];
			strcpy_s(tmpStr1, 256, airport->Name);
			strcat_s(tmpStr1, 256, ", ");
			strcat_s(tmpStr1, 256, LFIATAGetCountry(airport->CountryID)->Name);

			// Dateiname in ANSI
			CHAR tmpStr2[256];
			strcpy_s(tmpStr2, 256, airport->Code);
			strcat_s(tmpStr2, 256, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? "—" :" – ");
			strcat_s(tmpStr2, 256, tmpStr1);

			// Dateiname in Unicode
			WCHAR tmpStr3[256];
			MultiByteToWideChar(CP_ACP, 0, tmpStr2, -1, tmpStr3, 256);
			SetAttribute(folder, LFAttrFileName, tmpStr3);

			// Ortsname in Unicode
			MultiByteToWideChar(CP_ACP, 0, tmpStr1, -1, tmpStr3, 256);
			SetAttribute(folder, LFAttrLocationName, tmpStr3);

			// Weitere Attribute
			SetAttribute(folder, LFAttrLocationIATA, airport->Code);
			SetAttribute(folder, LFAttrLocationGPS, &airport->Location);

			return;
		}
	}

	CCategorizer::CustomizeFolder(folder, i);
}


// SizeCategorizer
//

SizeCategorizer::SizeCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL SizeCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeSize);

	return GetSizeCategory(*((INT64*)i1->AttributeValues[m_Attr]))==GetSizeCategory(*((INT64*)i2->AttributeValues[m_Attr]));
}

void SizeCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		WCHAR Name[256];
		LoadString(LFCoreModuleHandle, IDS_SIZE1+GetSizeCategory(*((INT64*)i->AttributeValues[m_Attr])), Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, m_Attr, i->AttributeValues[m_Attr]);
	}
}


// DurationCategorizer
//

DurationCategorizer::DurationCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL DurationCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeDuration);

	return GetDurationCategory(*((UINT*)i1->AttributeValues[m_Attr]))==GetDurationCategory(*((UINT*)i2->AttributeValues[m_Attr]));
}

void DurationCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		WCHAR Name[256];
		LoadString(LFCoreModuleHandle, IDS_DURATION1+GetDurationCategory(*((UINT*)i->AttributeValues[m_Attr])), Name, 256);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, m_Attr, i->AttributeValues[m_Attr]);
	}
}


// CNameCategorizer
//

CNameCategorizer::CNameCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL CNameCategorizer::GetNamePrefix(WCHAR* FullName, WCHAR* Buffer)
{
#define CHOOSE if ((P2) && ((!P1) || (P2<P1))) P1 = P2;

	WCHAR* P1 = wcsstr(FullName, L" —");
	WCHAR* P2;

	P2 = wcsstr(FullName, L" –"); CHOOSE;
	P2 = wcsstr(FullName, L" -"); CHOOSE;
	P2 = wcsstr(FullName, L" \""); CHOOSE;
	P2 = wcsstr(FullName, L" ("); CHOOSE;
	P2 = wcsstr(FullName, L" /"); CHOOSE;
	P2 = wcsstr(FullName, L" »"); CHOOSE;
	P2 = wcsstr(FullName, L" «"); CHOOSE;
	P2 = wcsstr(FullName, L" „"); CHOOSE;
	P2 = wcsstr(FullName, L" “"); CHOOSE;
	P2 = wcsstr(FullName, L"—"); CHOOSE;

	// Wenn kein Trenner gefunden wurde, von rechts nach Ziffern suchen
	if (!P1)
	{
		BYTE Stelle = 1;

		P2 = &FullName[wcslen(FullName)-1];
		while (P2>FullName)
			switch (Stelle)
			{
			case 1:
				switch (*P2)
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
				case L'.':
				case L',':
					P2--;
					break;
				case L' ':
					P2--;
					Stelle = 2;
					break;
				default:
					goto Skip;
				}
				break;
			case 2:
				if (*P2==L' ')
				{
					P2--;
				}
				else
				{
					goto Fertig;
				}
			}

Fertig:
		if (Stelle==2)
			P1 = P2+1;
	}

Skip:
	if (P1)
		wcsncpy_s(Buffer, 256, FullName, P1-FullName);

	return (P1!=NULL);
}

BOOL CNameCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeUnicodeString);

	WCHAR Prefix1[256];
	WCHAR Prefix2[256];

	BOOL res1 = GetNamePrefix((WCHAR*)i1->AttributeValues[m_Attr], Prefix1);
	BOOL res2 = GetNamePrefix((WCHAR*)i2->AttributeValues[m_Attr], Prefix2);

	return (res1 & res2) ? wcscmp(Prefix1, Prefix2)==0 : FALSE;
}

void CNameCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		WCHAR Name[256];
		if (!GetNamePrefix((WCHAR*)i->AttributeValues[m_Attr], &Name[0]))
			wcscpy_s(Name, 256, i->CoreAttributes.FileName);

		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, m_Attr, Name);
	}
}

LFFilterCondition* CNameCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = m_Attr;
	c->AttrData.Type = AttrTypes[m_Attr];
	c->AttrData.IsNull = FALSE;

	if (!GetNamePrefix((WCHAR*)i->AttributeValues[m_Attr], c->AttrData.UnicodeString))
		LFGetAttributeVariantData(i, c->AttrData);

	return c;
}


// MegapixelCategorizer
//

MegapixelCategorizer::MegapixelCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

void MegapixelCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		UINT dimension = (UINT)*((DOUBLE*)i->AttributeValues[m_Attr]);
		DOUBLE d = dimension;

		WCHAR Name[256];
		swprintf_s(Name, 256, L"%u Megapixel", dimension);
		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, m_Attr, &d);
	}
}

LFFilterCondition* MegapixelCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;
	c->AttrData.Attr = m_Attr;
	c->AttrData.Type = AttrTypes[m_Attr];
	c->AttrData.IsNull = FALSE;

	c->AttrData.Megapixel = (UINT)*((DOUBLE*)i->AttributeValues[m_Attr]);

	return c;
}


// URLCategorizer
//

URLCategorizer::URLCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL URLCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeAnsiString);

	CHAR Server1[256];
	CHAR Server2[256];

	GetServer((CHAR*)i1->AttributeValues[m_Attr], Server1);
	GetServer((CHAR*)i2->AttributeValues[m_Attr], Server2);

	return strcmp(Server1, Server2)==0;
}

void URLCategorizer::CustomizeFolder(LFItemDescriptor* folder, LFItemDescriptor* i)
{
	if (i->AttributeValues[m_Attr])
	{
		CHAR Server[256];
		GetServer((CHAR*)i->AttributeValues[m_Attr], Server);

		WCHAR Name[256];
		MultiByteToWideChar(CP_ACP, 0, Server, -1, Name, 256);

		SetAttribute(folder, LFAttrFileName, Name);
		SetAttribute(folder, m_Attr, Server);
	}
}

LFFilterCondition* URLCategorizer::GetCondition(LFItemDescriptor* i)
{
	LFFilterCondition* c = LFAllocFilterCondition();
	c->Compare = LFFilterCompareSubfolder;

	c->AttrData.Attr = m_Attr;
	c->AttrData.Type = AttrTypes[m_Attr];
	c->AttrData.IsNull = FALSE;

	GetServer((CHAR*)i->AttributeValues[m_Attr], c->AttrData.AnsiString);

	return c;
}
