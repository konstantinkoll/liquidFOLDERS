
#include "stdafx.h"
#include "Categorizers.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Query.h"


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

LFItemDescriptor* CCategorizer::GetFolder(LFItemDescriptor* i, LFFilter* pFilter)
{
	LFItemDescriptor* pFolder = AllocFolderDescriptor();

	if (i->AttributeValues[m_Attr])
	{
		pFolder->NextFilter = LFAllocFilter(pFilter);
		pFolder->NextFilter->Options.IsSubfolder = TRUE;
		pFolder->NextFilter->Options.GroupAttribute = m_Attr;
		pFolder->NextFilter->ConditionList = GetCondition(i, pFolder->NextFilter->ConditionList);

		// CustomizeFolder benötigt Filter-Bedingung aus GetCondition()
		CustomizeFolder(pFolder, i);

		wcscpy_s(pFolder->NextFilter->OriginalName, 256, pFolder->CoreAttributes.FileName);
	}

	return pFolder;
}

BOOL CCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	return CompareValues(AttrTypes[m_Attr], i1->AttributeValues[m_Attr], i2->AttributeValues[m_Attr], m_Attr>0)==0;
}

LFFilterCondition* CCategorizer::GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext)
{
	LFFilterCondition* c = LFAllocFilterConditionEx(m_Attr ? LFFilterCompareSubfolder : LFFilterCompareIsEqual, m_Attr, pNext);

	LFGetAttributeVariantData(i, c->AttrData);

	return c;
}

void CCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* /*i*/)
{
	LFSetAttributeVariantData(pFolder, pFolder->NextFilter->ConditionList->AttrData);

	if (m_Attr!=LFAttrFileName)
	{
		WCHAR Name[256];
		LFVariantDataToString(pFolder->NextFilter->ConditionList->AttrData, Name, 256);

		SetAttribute(pFolder, LFAttrFileName, Name);
	}
}


// CNameCategorizer
//

CNameCategorizer::CNameCategorizer()
	: CCategorizer(LFAttrFileName)
{
}

BOOL CNameCategorizer::GetNamePrefix(WCHAR* FullName, WCHAR* pBuffer)
{
#define CHOOSE if ((P2) && ((!P1) || (P2<P1))) P1 = P2;

	// Am weitesten links stehenden Trenner finden
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
		UINT Zustand = 1;

		P2 = &FullName[wcslen(FullName)-1];
		while (P2>FullName)
			switch (Zustand)
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
					Zustand = 2;
					break;

				default:
					goto Ende;
				}

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
		if (Zustand==2)
			P1 = P2+1;
	}

Ende:
	if (P1)
		wcsncpy_s(pBuffer, 256, FullName, P1-FullName);

	return (P1!=NULL);
}

BOOL CNameCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeUnicodeString);

	WCHAR Prefix1[256];
	BOOL Result1 = GetNamePrefix((WCHAR*)i1->AttributeValues[m_Attr], Prefix1);

	WCHAR Prefix2[256];
	BOOL Result2 = GetNamePrefix((WCHAR*)i2->AttributeValues[m_Attr], Prefix2);

	return (Result1 & Result2) ? wcscmp(Prefix1, Prefix2)==0 : FALSE;
}

LFFilterCondition* CNameCategorizer::GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext)
{
	LFFilterCondition* c = LFAllocFilterConditionEx(LFFilterCompareSubfolder, m_Attr, pNext);

	if (!GetNamePrefix((WCHAR*)i->AttributeValues[m_Attr], c->AttrData.UnicodeString))
		LFGetAttributeVariantData(i, c->AttrData);

	return c;
}


// CURLCategorizer
//

CURLCategorizer::CURLCategorizer()
	: CCategorizer(LFAttrURL)
{
}

void CURLCategorizer::GetServer(CHAR* URL, CHAR* Server, SIZE_T cCount)
{
	CHAR* Pos = strstr(URL, "://");
	if (Pos)
		URL = Pos+3;

	Pos = strchr(URL, '/');
	strncpy_s(Server, cCount, URL, Pos ? Pos-URL : cCount);
}

BOOL CURLCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeAnsiString);

	CHAR Server1[256];
	GetServer((CHAR*)i1->AttributeValues[m_Attr], Server1, 256);

	CHAR Server2[256];
	GetServer((CHAR*)i2->AttributeValues[m_Attr], Server2, 256);

	return strcmp(Server1, Server2)==0;
}

LFFilterCondition* CURLCategorizer::GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext)
{
	LFFilterCondition* c = LFAllocFilterConditionEx(LFFilterCompareSubfolder, m_Attr, pNext);

	GetServer((CHAR*)i->AttributeValues[m_Attr], c->AttrData.AnsiString, 256);

	return c;
}


// CIATACategorizer
//

CIATACategorizer::CIATACategorizer()
	: CCategorizer(LFAttrLocationIATA)
{
}

void CIATACategorizer::CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i)
{
	assert(i->AttributeValues[m_Attr]);

	LFAirport* pAirport;
	if (LFIATAGetAirportByCode((CHAR*)i->AttributeValues[m_Attr], &pAirport))
	{
		// Ortsname in ANSI
		CHAR tmpStr1[256];
		strcpy_s(tmpStr1, 256, pAirport->Name);
		strcat_s(tmpStr1, 256, ", ");
		strcat_s(tmpStr1, 256, LFIATAGetCountry(pAirport->CountryID)->Name);

		// Dateiname in ANSI
		CHAR tmpStr2[256];
		strcpy_s(tmpStr2, 256, pAirport->Code);
		strcat_s(tmpStr2, 256, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? "—" :" – ");
		strcat_s(tmpStr2, 256, tmpStr1);

		// Dateiname in Unicode
		WCHAR tmpStr3[256];
		MultiByteToWideChar(CP_ACP, 0, tmpStr2, -1, tmpStr3, 256);
		SetAttribute(pFolder, LFAttrFileName, tmpStr3);

		// Ortsname in Unicode
		MultiByteToWideChar(CP_ACP, 0, tmpStr1, -1, tmpStr3, 256);
		SetAttribute(pFolder, LFAttrLocationName, tmpStr3);

		// Weitere Attribute
		SetAttribute(pFolder, LFAttrLocationIATA, pAirport->Code);
		SetAttribute(pFolder, LFAttrLocationGPS, &pAirport->Location);
	}
	else
	{
		CCategorizer::CustomizeFolder(pFolder, i);
	}
}


// CRatingCategorizer
//

CRatingCategorizer::CRatingCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

__forceinline BYTE CRatingCategorizer::GetRatingCategory(const BYTE Rating)
{
	return (Rating==1) ? 1 : Rating>>1;
}

BOOL CRatingCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeRating);

	return GetRatingCategory(*((BYTE*)i1->AttributeValues[m_Attr]))==GetRatingCategory(*((BYTE*)i2->AttributeValues[m_Attr]));
}

void CRatingCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i)
{
	assert(i->AttributeValues[m_Attr]);

	BYTE Rating = GetRatingCategory(*((BYTE*)i->AttributeValues[m_Attr]));

	WCHAR Name[256];
	LoadString(LFCoreModuleHandle, ((m_Attr==LFAttrRating) ? IDS_RATING1 : IDS_PRIORITY1)+Rating-1, Name, 256);
	SetAttribute(pFolder, LFAttrFileName, Name);

	Rating *= 2;
	SetAttribute(pFolder, m_Attr, &Rating);
}


// CSizeCategorizer
//

CSizeCategorizer::CSizeCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

UINT CSizeCategorizer::GetSizeCategory(const INT64 Size)
{
	return (Size<32*1024) ? 0 : (Size<128*1024) ? 1 : (Size<1024*1024) ? 2: (Size<16384*1024) ? 3 : (Size<131072*1024) ? 4 : 5;
}

BOOL CSizeCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeSize);

	return GetSizeCategory(*((INT64*)i1->AttributeValues[m_Attr]))==GetSizeCategory(*((INT64*)i2->AttributeValues[m_Attr]));
}

void CSizeCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i)
{
	assert(i->AttributeValues[m_Attr]);

	WCHAR Name[256];
	LoadString(LFCoreModuleHandle, IDS_SIZE1+GetSizeCategory(*((INT64*)i->AttributeValues[m_Attr])), Name, 256);
	SetAttribute(pFolder, LFAttrFileName, Name);

	SetAttribute(pFolder, m_Attr, i->AttributeValues[m_Attr]);
}


// CDateCategorizer
//

CDateCategorizer::CDateCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

void CDateCategorizer::GetDate(const FILETIME* Time, LPSYSTEMTIME Date)
{
	SYSTEMTIME stUTC;
	FileTimeToSystemTime(Time, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, Date);
}

void CDateCategorizer::GetDay(const FILETIME* Time, LPFILETIME Day)
{
	SYSTEMTIME stLocal;
	GetDate(Time, &stLocal);

	stLocal.wHour = stLocal.wMinute = stLocal.wSecond = stLocal.wMilliseconds = 0;

	SYSTEMTIME stUTC;
	TzSpecificLocalTimeToSystemTime(NULL, &stLocal, &stUTC);
	SystemTimeToFileTime(&stUTC, Day);
}

BOOL CDateCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeTime);

	SYSTEMTIME stLocal1;
	GetDate((FILETIME*)i1->AttributeValues[m_Attr], &stLocal1);

	SYSTEMTIME stLocal2;
	GetDate((FILETIME*)i2->AttributeValues[m_Attr], &stLocal2);

	return (stLocal1.wDay==stLocal2.wDay) && (stLocal1.wMonth==stLocal2.wMonth) && (stLocal1.wYear==stLocal2.wYear);
}

LFFilterCondition* CDateCategorizer::GetCondition(LFItemDescriptor* i, LFFilterCondition* pNext)
{
	LFFilterCondition* c = LFAllocFilterConditionEx(LFFilterCompareSubfolder, m_Attr, pNext);

	GetDay((FILETIME*)i->AttributeValues[m_Attr], &c->AttrData.Time);

	return c;
}


// CDurationCategorizer
//

CDurationCategorizer::CDurationCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

UINT CDurationCategorizer::GetDurationCategory(const UINT Duration)
{
	if (Duration<5*1000)
		return 0;

	if (Duration<15*1000)
		return 1;

	if (Duration<30*1000)
		return 2;

	if (Duration<1*60*1000)
		return 3;

	if (Duration<2*60*1000)
		return 4;

	if (Duration<3*60*1000)
		return 5;

	if (Duration<5*60*1000)
		return 6;

	if (Duration<15*60*1000)
		return 7;

	if (Duration<30*60*1000)
		return 8;

	if (Duration<45*60*1000)
		return 9;

	if (Duration<60*60*1000)
		return 10;

	if (Duration<90*60*1000)
		return 11;

	if (Duration<120*60*1000)
		return 12;

	return 13;
}

BOOL CDurationCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeDuration);

	return GetDurationCategory(*((UINT*)i1->AttributeValues[m_Attr]))==GetDurationCategory(*((UINT*)i2->AttributeValues[m_Attr]));
}

void CDurationCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i)
{
	assert(i->AttributeValues[m_Attr]);

	WCHAR Name[256];
	LoadString(LFCoreModuleHandle, IDS_DURATION1+GetDurationCategory(*((UINT*)i->AttributeValues[m_Attr])), Name, 256);
	SetAttribute(pFolder, LFAttrFileName, Name);

	SetAttribute(pFolder, m_Attr, i->AttributeValues[m_Attr]);
}


// CMegapixelCategorizer
//

CMegapixelCategorizer::CMegapixelCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL CMegapixelCategorizer::CompareItems(LFItemDescriptor* i1, LFItemDescriptor* i2)
{
	assert(AttrTypes[m_Attr]==LFTypeMegapixel);

	return (UINT)*(DOUBLE*)i1->AttributeValues[m_Attr]==(UINT)*(DOUBLE*)i2->AttributeValues[m_Attr];
}


void CMegapixelCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, LFItemDescriptor* i)
{
	assert(i->AttributeValues[m_Attr]);

	UINT Dimension = (UINT)*(DOUBLE*)i->AttributeValues[m_Attr];

	WCHAR Name[256];
	swprintf_s(Name, 256, L"%u Megapixel", Dimension);
	SetAttribute(pFolder, LFAttrFileName, Name);

	DOUBLE d = Dimension;
	SetAttribute(pFolder, m_Attr, &d);
}
