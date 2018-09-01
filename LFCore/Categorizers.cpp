
#include "stdafx.h"
#include "Categorizers.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Query.h"
#include "TableAttributes.h"


extern HMODULE LFCoreModuleHandle;
extern WCHAR ItemColorNames[LFItemColorCount-1][256];


// CCategorizer
//

CCategorizer::CCategorizer(UINT Attr)
{
	m_Attr = Attr;
}

BOOL CCategorizer::IsEqual(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	if (!pItemDescriptor1->AttributeValues[m_Attr] || !pItemDescriptor2->AttributeValues[m_Attr])
		return pItemDescriptor1->AttributeValues[m_Attr]==pItemDescriptor2->AttributeValues[m_Attr];

	return CompareItems(pItemDescriptor1, pItemDescriptor2);
}

LFItemDescriptor* CCategorizer::GetFolder(LFItemDescriptor* pItemDescriptor, LFFilter* pFilter, LFFileSummary& FileSummary, INT FirstAggregate, INT LastAggregate) const
{
	assert(pItemDescriptor);
	assert(pItemDescriptor->AttributeValues[m_Attr]);

	// Filter condition
	LFVariantData VData;
	GetFilterValue(VData, pItemDescriptor);

	// Folder
	LFItemDescriptor* pFolder = AllocFolderDescriptor(FileSummary, VData, pFilter, FirstAggregate, LastAggregate);

	CustomizeFolder(pFolder, pItemDescriptor);
	wcscpy_s(pFolder->pNextFilter->Name, 256, pFolder->CoreAttributes.FileName);

	// No folder name for colors
	if (AttrProperties[m_Attr].Type==LFTypeColor)
		pFolder->CoreAttributes.FileName[0] = L'\0';

	return pFolder;
}

BOOL CCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	return CompareValues(AttrProperties[m_Attr].Type, pItemDescriptor1->AttributeValues[m_Attr], pItemDescriptor2->AttributeValues[m_Attr], m_Attr>0)==0;
}

void CCategorizer::GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const
{
	LFGetAttributeVariantDataEx(pItemDescriptor, m_Attr, VData);
}

void CCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* /*pSpecimen*/) const
{
	LFSetAttributeVariantData(pFolder, pFolder->pNextFilter->Query.pConditionList->VData);

	if (m_Attr!=LFAttrFileName)
	{
		const LFVariantData* pVData = &pFolder->pNextFilter->Query.pConditionList->VData;

		// Set category name as file name
		WCHAR Name[256];
		LFVariantDataToString(*pVData, Name, 256);

		SetAttribute(pFolder, LFAttrFileName, Name);
	}
}


// CNameCategorizer
//

CNameCategorizer::CNameCategorizer()
	: CCategorizer(LFAttrFileName)
{
}

BOOL CNameCategorizer::GetNamePrefix(LPCWSTR FullName, LPWSTR pStr, SIZE_T cCount)
{
#define CHOOSE if (pChar2 && (!pChar1 || (pChar2<pChar1))) pChar1 = pChar2;

	// Am weitesten links stehenden Trenner finden
	LPCWSTR pChar1 = wcsstr(FullName, L" —");
	LPCWSTR pChar2;

	pChar2 = wcsstr(FullName, L" –"); CHOOSE;
	pChar2 = wcsstr(FullName, L" -"); CHOOSE;
	pChar2 = wcsstr(FullName, L" \""); CHOOSE;
	pChar2 = wcsstr(FullName, L" ("); CHOOSE;
	pChar2 = wcsstr(FullName, L" /"); CHOOSE;
	pChar2 = wcsstr(FullName, L" »"); CHOOSE;
	pChar2 = wcsstr(FullName, L" «"); CHOOSE;
	pChar2 = wcsstr(FullName, L" „"); CHOOSE;
	pChar2 = wcsstr(FullName, L" “"); CHOOSE;
	pChar2 = wcsstr(FullName, L"—"); CHOOSE;

	// Wenn kein Trenner gefunden wurde, von rechts nach Ziffern suchen
	if (!pChar1)
	{
		UINT Zustand = 1;

		pChar2 = &FullName[wcslen(FullName)-1];
		while (pChar2>FullName)
			switch (Zustand)
			{
			case 1:
				switch (*pChar2)
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
					pChar2--;
					break;

				case L' ':
					pChar2--;
					Zustand = 2;
					break;

				default:
					goto Ende;
				}

			case 2:
				if (*pChar2==L' ')
				{
					pChar2--;
				}
				else
				{
					goto Fertig;
				}
			}

Fertig:
		if (Zustand==2)
			pChar1 = pChar2+1;
	}

Ende:
	if (pChar1)
		wcsncpy_s(pStr, cCount, FullName, pChar1-FullName);

	return (pChar1!=NULL);
}

BOOL CNameCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeUnicodeString);

	WCHAR Prefix1[256];
	BOOL Result1 = GetNamePrefix((LPCWSTR)pItemDescriptor1->AttributeValues[m_Attr], Prefix1, 256);

	WCHAR Prefix2[256];
	BOOL Result2 = GetNamePrefix((LPCWSTR)pItemDescriptor2->AttributeValues[m_Attr], Prefix2, 256);

	return (Result1 & Result2) ? wcscmp(Prefix1, Prefix2)==0 : FALSE;
}

void CNameCategorizer::GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const
{
	LFInitVariantData(VData, m_Attr);

	if (!GetNamePrefix((LPCWSTR)pItemDescriptor->AttributeValues[m_Attr], VData.UnicodeString, 256))
		LFGetAttributeVariantData(pItemDescriptor, VData);
}


// CURLCategorizer
//

CURLCategorizer::CURLCategorizer()
	: CCategorizer(LFAttrURL)
{
}

void CURLCategorizer::GetServer(LPCSTR URL, LPSTR pStr, SIZE_T cCount)
{
	LPCSTR Pos = strstr(URL, "://");
	if (Pos)
		URL = Pos+3;

	Pos = strchr(URL, '/');
	strncpy_s(pStr, cCount, URL, Pos ? Pos-URL : cCount);
}

BOOL CURLCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeAnsiString);

	CHAR Server1[256];
	GetServer((LPCSTR)pItemDescriptor1->AttributeValues[m_Attr], Server1, 256);

	CHAR Server2[256];
	GetServer((LPCSTR)pItemDescriptor2->AttributeValues[m_Attr], Server2, 256);

	return strcmp(Server1, Server2)==0;
}

void CURLCategorizer::GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const
{
	LFInitVariantData(VData, m_Attr);

	GetServer((LPCSTR)pItemDescriptor->AttributeValues[m_Attr], VData.AnsiString, 256);
	VData.IsNull = FALSE;
}


// CIATACategorizer
//

CIATACategorizer::CIATACategorizer()
	: CCategorizer(LFAttrLocationIATA)
{
	m_Separator = (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? L"—" : L" – ";
}

void CIATACategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const
{
	assert(pSpecimen->AttributeValues[m_Attr]);
	assert(AttrProperties[m_Attr].Type==LFTypeIATACode);

	LFAirport* pAirport;
	if (LFIATAGetAirportByCode((LPCSTR)pSpecimen->AttributeValues[m_Attr], pAirport))
	{
		// Location name
		WCHAR LocationName[256];
		LFIATAGetLocationNameForAirport(pAirport, LocationName, 256);

		// IATA code to Unicode
		assert(strlen(pAirport->Code)==3);
		pFolder->FolderMainCaptionCount = 3;

		WCHAR tmpStr[256];
		MultiByteToWideChar(CP_ACP, 0, pAirport->Code, 4, tmpStr, 256);

		wcscat_s(tmpStr, 256, m_Separator);

		pFolder->FolderSubcaptionStart = (UINT)wcslen(tmpStr);
		wcscat_s(tmpStr, 256, LocationName);

		// Set attributes
		SetAttribute(pFolder, LFAttrFileName, tmpStr);
		SetAttribute(pFolder, LFAttrLocationName, LocationName);
		SetAttribute(pFolder, LFAttrLocationIATA, pAirport->Code);
		SetAttribute(pFolder, LFAttrLocationGPS, &pAirport->Location);
	}
	else
	{
		CCategorizer::CustomizeFolder(pFolder, pSpecimen);
	}
}


// CChannelsCategorizer
//

CChannelsCategorizer::CChannelsCategorizer()
	: CCategorizer(LFAttrChannels)
{
}

__forceinline UINT CChannelsCategorizer::GetChannelsCategory(const UINT Channels)
{
	return (Channels>2) ? 2 : Channels-1;
}

BOOL CChannelsCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeUINT);

	return GetChannelsCategory(*((UINT*)pItemDescriptor1->AttributeValues[m_Attr]))==GetChannelsCategory(*((UINT*)pItemDescriptor2->AttributeValues[m_Attr]));
}

void CChannelsCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const
{
	assert(pSpecimen->AttributeValues[m_Attr]);
	assert(AttrProperties[m_Attr].Type==LFTypeUINT);

	WCHAR Name[256];
	LoadString(LFCoreModuleHandle, IDS_CHANNELS1+GetChannelsCategory(*((UINT*)pSpecimen->AttributeValues[m_Attr])), Name, 256);
	SetAttribute(pFolder, LFAttrFileName, Name);

	SetAttribute(pFolder, m_Attr, pSpecimen->AttributeValues[m_Attr]);
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

BOOL CRatingCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeRating);

	return GetRatingCategory(*((BYTE*)pItemDescriptor1->AttributeValues[m_Attr]))==GetRatingCategory(*((BYTE*)pItemDescriptor2->AttributeValues[m_Attr]));
}

void CRatingCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const
{
	assert(pSpecimen->AttributeValues[m_Attr]);
	assert(AttrProperties[m_Attr].Type==LFTypeRating);

	BYTE Rating = GetRatingCategory(*((BYTE*)pSpecimen->AttributeValues[m_Attr]));
	assert(Rating<=LFMaxRating);

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

BOOL CSizeCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeSize);

	return GetSizeCategory(*((INT64*)pItemDescriptor1->AttributeValues[m_Attr]))==GetSizeCategory(*((INT64*)pItemDescriptor2->AttributeValues[m_Attr]));
}

void CSizeCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const
{
	assert(pSpecimen->AttributeValues[m_Attr]);
	assert(AttrProperties[m_Attr].Type==LFTypeSize);

	WCHAR Name[256];
	LoadString(LFCoreModuleHandle, IDS_SIZE1+GetSizeCategory(*((INT64*)pSpecimen->AttributeValues[m_Attr])), Name, 256);
	SetAttribute(pFolder, LFAttrFileName, Name);

	SetAttribute(pFolder, m_Attr, pSpecimen->AttributeValues[m_Attr]);
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

BOOL CDateCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeTime);

	SYSTEMTIME stLocal1;
	GetDate((FILETIME*)pItemDescriptor1->AttributeValues[m_Attr], &stLocal1);

	SYSTEMTIME stLocal2;
	GetDate((FILETIME*)pItemDescriptor2->AttributeValues[m_Attr], &stLocal2);

	return (stLocal1.wDay==stLocal2.wDay) && (stLocal1.wMonth==stLocal2.wMonth) && (stLocal1.wYear==stLocal2.wYear);
}

void CDateCategorizer::GetFilterValue(LFVariantData& VData, LFItemDescriptor* pItemDescriptor) const
{
	LFInitVariantData(VData, m_Attr);

	GetDay((FILETIME*)pItemDescriptor->AttributeValues[m_Attr], &VData.Time);
	VData.IsNull = FALSE;
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

BOOL CDurationCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeDuration);

	return GetDurationCategory(*((UINT*)pItemDescriptor1->AttributeValues[m_Attr]))==GetDurationCategory(*((UINT*)pItemDescriptor2->AttributeValues[m_Attr]));
}

void CDurationCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const
{
	assert(pSpecimen->AttributeValues[m_Attr]);
	assert(AttrProperties[m_Attr].Type==LFTypeDuration);

	WCHAR Name[256];
	LoadString(LFCoreModuleHandle, IDS_DURATION1+GetDurationCategory(*((UINT*)pSpecimen->AttributeValues[m_Attr])), Name, 256);
	SetAttribute(pFolder, LFAttrFileName, Name);

	SetAttribute(pFolder, m_Attr, pSpecimen->AttributeValues[m_Attr]);
}


// CMegapixelCategorizer
//

CMegapixelCategorizer::CMegapixelCategorizer(UINT Attr)
	: CCategorizer(Attr)
{
}

BOOL CMegapixelCategorizer::CompareItems(const LFItemDescriptor* pItemDescriptor1, const LFItemDescriptor* pItemDescriptor2) const
{
	assert(AttrProperties[m_Attr].Type==LFTypeMegapixel);

	return (UINT)*(DOUBLE*)pItemDescriptor1->AttributeValues[m_Attr]==(UINT)*(DOUBLE*)pItemDescriptor2->AttributeValues[m_Attr];
}

void CMegapixelCategorizer::CustomizeFolder(LFItemDescriptor* pFolder, const LFItemDescriptor* pSpecimen) const
{
	assert(pSpecimen->AttributeValues[m_Attr]);
	assert(AttrProperties[m_Attr].Type==LFTypeMegapixel);

	const UINT Dimension = (UINT)*(DOUBLE*)pSpecimen->AttributeValues[m_Attr];

	WCHAR Name[256];
	swprintf_s(Name, 256, L"%u Megapixel", Dimension);
	SetAttribute(pFolder, LFAttrFileName, Name);

	DOUBLE d = Dimension;
	SetAttribute(pFolder, m_Attr, &d);
}
