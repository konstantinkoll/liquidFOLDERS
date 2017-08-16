
#include "stdafx.h"
#include "AttributeTables.h"
#include "Categorizers.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Query.h"
#include "Stores.h"
#include <assert.h>
#include <shlwapi.h>


#pragma comment(lib, "shlwapi.lib")


BOOL CheckCondition(LPVOID pValue, LFFilterCondition* pFilterCondition)
{
	assert(pFilterCondition);
	assert(pFilterCondition->Compare>=LFFilterCompareIgnore);
	assert(pFilterCondition->Compare<=LFFilterCompareContains);
	assert(pFilterCondition->AttrData.Attr<LFAttributeCount);
	assert(pFilterCondition->AttrData.Type==AttrProperties[pFilterCondition->AttrData.Attr].Type);
	assert(pFilterCondition->AttrData.Type<LFTypeCount);

	switch (pFilterCondition->Compare)
	{
	case LFFilterCompareIgnore:
		return TRUE;

	case LFFilterCompareIsNull:
		return IsNullValue(pFilterCondition->AttrData.Type, pValue);
	}

	if (!pValue)
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return LFIsNullVariantData(pFilterCondition->AttrData);

		case LFFilterCompareIsNotEqual:
			return !LFIsNullVariantData(pFilterCondition->AttrData);

		default:
			return FALSE;
		}

	SIZE_T Len1;
	SIZE_T Len2;
	CHAR Server[256];
	FILETIME Time1;
	FILETIME Time2;
	ULARGE_INTEGER ULI1;
	ULARGE_INTEGER ULI2;
	LPCWSTR pHashtagArray;
	WCHAR Hashtag[256];

	switch (pFilterCondition->AttrData.Type)
	{
	case LFTypeUnicodeString:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			if (pFilterCondition->AttrData.Attr==LFAttrFileName)
			{
				Len1 = wcslen((LPCWSTR)pValue);
				Len2 = wcslen(pFilterCondition->AttrData.UnicodeString);

				return (Len1<=Len2) ? FALSE : _wcsnicmp((LPCWSTR)pValue, pFilterCondition->AttrData.UnicodeString, Len2)==0;
			}

		case LFFilterCompareIsEqual:
			return _wcsicmp((LPCWSTR)pValue, pFilterCondition->AttrData.UnicodeString)==0;

		case LFFilterCompareIsNotEqual:
			return _wcsicmp((LPCWSTR)pValue, pFilterCondition->AttrData.UnicodeString)!=0;
		case LFFilterCompareBeginsWith:
			Len1 = wcslen(pFilterCondition->AttrData.UnicodeString);
			Len2 = wcslen((LPCWSTR)pValue);

			return (Len1>Len2) ? FALSE :_wcsnicmp((LPCWSTR)pValue, pFilterCondition->AttrData.UnicodeString, Len1)==0;

		case LFFilterCompareEndsWith:
			Len1 = wcslen(pFilterCondition->AttrData.UnicodeString);
			Len2 = wcslen((LPCWSTR)pValue);

			return (Len1>Len2) ? FALSE : _wcsicmp(&((LPCWSTR)pValue)[Len2-Len1], pFilterCondition->AttrData.UnicodeString)==0;

		case LFFilterCompareContains:
			return StrStrIW((LPCWSTR)pValue, pFilterCondition->AttrData.UnicodeString)!=NULL;

		default:
			return FALSE;
		}

	case LFTypeUnicodeArray:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			return LFContainsHashtag((LPCWSTR)pValue, pFilterCondition->AttrData.UnicodeString);

		case LFFilterCompareContains:
			pHashtagArray = pFilterCondition->AttrData.UnicodeArray;
			while (GetNextHashtag(&pHashtagArray, Hashtag, 256))
				if (LFContainsHashtag((LPCWSTR)pValue, Hashtag))
					return TRUE;

			return FALSE;

		default:
			return FALSE;
		}

	case LFTypeAnsiString:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			if (pFilterCondition->AttrData.Attr==LFAttrURL)
			{
				CURLCategorizer::GetServer((LPCSTR)pValue, Server, 256);

				return _stricmp(Server, pFilterCondition->AttrData.AnsiString)==0;
			}

		case LFFilterCompareIsEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->AttrData.AnsiString)==0;

		case LFFilterCompareIsNotEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->AttrData.AnsiString)!=0;

		case LFFilterCompareBeginsWith:
			Len1 = strlen(pFilterCondition->AttrData.AnsiString);
			Len2 = strlen((LPCSTR)pValue);

			return (Len1>Len2) ? FALSE : _strnicmp((LPCSTR)pValue, pFilterCondition->AttrData.AnsiString, Len1)==0;

		case LFFilterCompareEndsWith:
			Len1 = strlen(pFilterCondition->AttrData.AnsiString);
			Len2 = strlen((LPCSTR)pValue);

			return (Len1>Len2) ? FALSE : _stricmp(&((LPCSTR)pValue)[Len2-Len1], pFilterCondition->AttrData.AnsiString)==0;

		case LFFilterCompareContains:
			return StrStrIA((LPCSTR)pValue, pFilterCondition->AttrData.AnsiString)!=NULL;

		default:
			return FALSE;
		}

	case LFTypeIATACode:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->AttrData.IATAString)==0;

		case LFFilterCompareIsNotEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->AttrData.IATAString)!=0;

		default:
			return FALSE;
		}

	case LFTypeFourCC:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(UINT*)pValue==pFilterCondition->AttrData.UINT32;

		case LFFilterCompareIsNotEqual:
			return *(UINT*)pValue!=pFilterCondition->AttrData.UINT32;

		default:
			return FALSE;
		}

	case LFTypeRating:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			return CRatingCategorizer::GetRatingCategory(*(BYTE*)pValue)==CRatingCategorizer::GetRatingCategory(pFilterCondition->AttrData.Rating);

		case LFFilterCompareIsEqual:
			return *(BYTE*)pValue==pFilterCondition->AttrData.Rating;

		case LFFilterCompareIsNotEqual:
			return *(BYTE*)pValue!=pFilterCondition->AttrData.Rating;

		case LFFilterCompareIsAboveOrEqual:
			return *(BYTE*)pValue>=pFilterCondition->AttrData.Rating;

		case LFFilterCompareIsBelowOrEqual:
			return *(BYTE*)pValue<=pFilterCondition->AttrData.Rating;

		default:
			return FALSE;
		}

	case LFTypeUINT:
	case LFTypeDuration:
	case LFTypeGenre:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			if (pFilterCondition->AttrData.Type==LFTypeDuration)
				return CDurationCategorizer::GetDurationCategory(*(UINT*)pValue)==CDurationCategorizer::GetDurationCategory(pFilterCondition->AttrData.UINT32);

		case LFFilterCompareIsEqual:
			return *(UINT*)pValue==pFilterCondition->AttrData.UINT32;

		case LFFilterCompareIsNotEqual:
			return *(UINT*)pValue!=pFilterCondition->AttrData.UINT32;

		case LFFilterCompareIsAboveOrEqual:
			return *(UINT*)pValue>=pFilterCondition->AttrData.UINT32;

		case LFFilterCompareIsBelowOrEqual:
			return *(UINT*)pValue<=pFilterCondition->AttrData.UINT32;

		default:
			return FALSE;
		}

	case LFTypeSize:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			return CSizeCategorizer::GetSizeCategory(*(INT64*)pValue)==CSizeCategorizer::GetSizeCategory(pFilterCondition->AttrData.INT64);

		case LFFilterCompareIsEqual:
			return *(INT64*)pValue==pFilterCondition->AttrData.INT64;

		case LFFilterCompareIsNotEqual:
			return *(INT64*)pValue!=pFilterCondition->AttrData.INT64;

		case LFFilterCompareIsAboveOrEqual:
			return *(INT64*)pValue>=pFilterCondition->AttrData.INT64;

		case LFFilterCompareIsBelowOrEqual:
			return *(INT64*)pValue<=pFilterCondition->AttrData.INT64;

		default:
			return FALSE;
		}

	case LFTypeFraction:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(pValue, &pFilterCondition->AttrData.Fraction, sizeof(LFFraction))==0;

		case LFFilterCompareIsNotEqual:
			return memcmp(pValue, &pFilterCondition->AttrData.Fraction, sizeof(LFFraction))!=0;

		default:
			return FALSE;
		}

	case LFTypeDouble:
	case LFTypeMegapixel:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(DOUBLE*)pValue==pFilterCondition->AttrData.Double;

		case LFFilterCompareIsNotEqual:
			return *(DOUBLE*)pValue!=pFilterCondition->AttrData.Double;

		case LFFilterCompareIsAboveOrEqual:
			return *(DOUBLE*)pValue>=pFilterCondition->AttrData.Double;

		case LFFilterCompareIsBelowOrEqual:
			return *(DOUBLE*)pValue<=pFilterCondition->AttrData.Double;

		default:
			return FALSE;
		}

	case LFTypeGeoCoordinates:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(pValue, &pFilterCondition->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))==0;

		case LFFilterCompareIsNotEqual:
			return memcmp(pValue, &pFilterCondition->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))!=0;

		default:
			return FALSE;
		}

	case LFTypeTime:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			CDateCategorizer::GetDay((FILETIME*)pValue, &Time1);
			CDateCategorizer::GetDay(&pFilterCondition->AttrData.Time, &Time2);

			return memcmp(&Time1, &Time2, sizeof(FILETIME))==0;

		case LFFilterCompareIsNotEqual:
			CDateCategorizer::GetDay((FILETIME*)pValue, &Time1);
			CDateCategorizer::GetDay(&pFilterCondition->AttrData.Time, &Time2);

			return memcmp(&Time1, &Time2, sizeof(FILETIME))!=0;

		case LFFilterCompareIsAboveOrEqual:
			ULI1.LowPart = (*(FILETIME*)pValue).dwLowDateTime;
			ULI1.HighPart = (*(FILETIME*)pValue).dwHighDateTime;
			ULI2.LowPart = pFilterCondition->AttrData.Time.dwLowDateTime;
			ULI2.HighPart = pFilterCondition->AttrData.Time.dwHighDateTime;

			return ULI1.QuadPart>=ULI2.QuadPart;

		case LFFilterCompareIsBelowOrEqual:
			ULI1.LowPart = (*(FILETIME*)pValue).dwLowDateTime;
			ULI1.HighPart = (*(FILETIME*)pValue).dwHighDateTime;
			ULI2.LowPart = pFilterCondition->AttrData.Time.dwLowDateTime;
			ULI2.HighPart = pFilterCondition->AttrData.Time.dwHighDateTime;

			return ULI1.QuadPart<=ULI2.QuadPart;

		default:
			return FALSE;
		}

	case LFTypeBitrate:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return ((*(UINT*)pValue+500)/1000)==((pFilterCondition->AttrData.UINT32+500)/1000);

		case LFFilterCompareIsNotEqual:
			return ((*(UINT*)pValue+500)/1000)!=((pFilterCondition->AttrData.UINT32+500)/1000);

		case LFFilterCompareIsAboveOrEqual:
			return *(UINT*)pValue>=pFilterCondition->AttrData.UINT32;

		case LFFilterCompareIsBelowOrEqual:
			return *(UINT*)pValue<=pFilterCondition->AttrData.UINT32;

		default:
			return FALSE;
		}
	}

	return TRUE;
}

BOOL PassesFilter(UINT TableID, LPVOID pTableData, LFFilter* pFilter, BOOL& CheckSearchterm, BYTE& SearchtermContainsLetters)
{
	assert(TableID>=0);
	assert(TableID<IDXTABLECOUNT);
	assert(pTableData);
	assert(pFilter);

	// Primary checks on core attributes only
	if (TableID==IDXTABLE_MASTER)
	{
		const LFCoreAttributes* pCoreAttributes = (LFCoreAttributes*)pTableData;

		// Only show trashed files when filter queries trashcan
		if ((pCoreAttributes->Flags & LFFlagTrash) && (pFilter->QueryContext!=LFContextTrash))
			return FALSE;

		// Only show archived files when filter queries archive
		if ((pCoreAttributes->Flags & LFFlagArchive) && (pFilter->QueryContext!=LFContextArchive) &&
			(((pCoreAttributes->Flags & LFFlagTrash)==0) || (pFilter->QueryContext!=LFContextTrash)))
			return FALSE;

		if (((pFilter->QueryContext!=LFContextAllFiles) && (pFilter->QueryContext!=LFContextAuto)) || (pCoreAttributes->ContextID==LFContextFilters))
			switch (pFilter->QueryContext)
			{
			case LFContextFavorites:
				if (!pCoreAttributes->Rating)
					return FALSE;

				break;

			case LFContextTasks:
				if (!(pCoreAttributes->Flags & LFFlagTask) && (pCoreAttributes->ContextID!=LFContextTasks))
					return FALSE;

				break;

			case LFContextNew:
				if (!(pCoreAttributes->Flags & LFFlagNew))
					return FALSE;

				break;

			case LFContextArchive:
				if (!(pCoreAttributes->Flags & LFFlagArchive))
					return FALSE;

				break;

			case LFContextTrash:
				if (!(pCoreAttributes->Flags & LFFlagTrash))
					return FALSE;

				break;

			case LFContextFilters:
				if (pFilter->Options.IsPersistent)
					return FALSE;

			default:
				if (pFilter->QueryContext!=pCoreAttributes->ContextID)
					return FALSE;
			}
	}

	// Check table
	const IdxTable* pTable = &IndexTables[TableID];

	LFFilterCondition* pFilterCondition = pFilter->pConditionList;
	while (pFilterCondition)
	{
		for (UINT a=0; a<pTable->cTableEntries; a++)
			if (pTable->pTableEntries[a].Attr==pFilterCondition->AttrData.Attr)
				if (!CheckCondition((BYTE*)pTableData+pTable->pTableEntries[a].Offset, pFilterCondition))
					return FALSE;

		pFilterCondition = pFilterCondition->pNext;
	}

	// Searchterm
	if (pFilter->Searchterm[0]==L'\0')
		CheckSearchterm = TRUE;

	if (!CheckSearchterm)
	{
		// Check if searchterm contains letters: if yes, do not compare times
		if (!SearchtermContainsLetters)
		{
			SearchtermContainsLetters = 1;

			LPCWSTR pChar = pFilter->Searchterm;
			while (*pChar)
			{
				if (((*pChar>=L'A') && (*pChar<=L'Z')) || ((*pChar>=L'a') && (*pChar<=L'z')))
				{
					SearchtermContainsLetters = 2;
					break;
				}

				pChar++;
			}
		}

		// Compare attributes
		for (UINT a=0; a<pTable->cTableEntries; a++)
		{
			const UINT Attr = pTable->pTableEntries[a].Attr;

			if ((SearchtermContainsLetters<2) || TypeProperties[AttrProperties[Attr].Type].ContainsLetters)
			{
				WCHAR tmpStr[256];
				ToString((BYTE*)pTableData+pTable->pTableEntries[a].Offset, AttrProperties[pTable->pTableEntries[a].Attr].Type, tmpStr, 256);

				if (StrStrIW(tmpStr, pFilter->Searchterm)!=NULL)
				{
					CheckSearchterm = TRUE;
					break;
				}
			}
		}
	}

	return CheckSearchterm || ((TableID==IDXTABLE_MASTER) && (((LFCoreAttributes*)pTableData)->SlaveID!=0));
}

BOOL PassesFilter(LFItemDescriptor* pItemDescriptor, LFFilter* pFilter)
{
	assert(pItemDescriptor);
	assert(pFilter);

	LFFilterCondition* pFilterCondition = pFilter->pConditionList;
	while (pFilterCondition)
	{
		switch (pFilterCondition->AttrData.Attr)
		{
		case LFAttrDimension:
			if (!CheckCondition(&pItemDescriptor->Dimension, pFilterCondition))
				return FALSE;

			break;

		case LFAttrAspectRatio:
			if (!CheckCondition(&pItemDescriptor->AspectRatio, pFilterCondition))
				return FALSE;

			break;
		}

		pFilterCondition = pFilterCondition->pNext;
	}

	return TRUE;
}


// Query helpers
//

void QueryStore(LPCSTR StoreID, LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	CStore* pStore;

	if (OpenStore(StoreID, FALSE, &pStore)==LFOk)
		pStore->Query(pFilter, pSearchResult);

	delete pStore;
}

__forceinline void QueryTree(LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	QueryStore(pFilter->StoreID, pFilter, pSearchResult);
}

__forceinline void QuerySearch(LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	if (pFilter->StoreID[0])
	{
		// Single store
		QueryStore(pFilter->StoreID, pFilter, pSearchResult);
	}
	else
	{
		CHAR* pStoreIDs;
		UINT Count;
		if ((pSearchResult->m_LastError=LFGetAllStores(&pStoreIDs, &Count))!=LFOk)
			return;

		if (Count)
		{
			LPCSTR pChar = pStoreIDs;
			for (UINT a=0; a<Count; a++)
			{
				QueryStore(pChar, pFilter, pSearchResult);

				pChar += LFKeySize;
			}

			free(pStoreIDs);
		}
	}
}


// Public functions
//

LFCORE_API LFSearchResult* LFQuery(LFFilter* pFilter)
{
	DWORD Start = GetTickCount();

	LFSearchResult* pSearchResult = new LFSearchResult();

	if (!pFilter)
	{
		QueryStores(pSearchResult);
	}
	else
	{
		// Query
		switch (pFilter->Mode)
		{
		case LFFilterModeStores:
			QueryStores(pSearchResult);

			break;

		case LFFilterModeDirectoryTree:
			if ((pFilter->StoreID[0]=='\0') && (pFilter->Mode==LFFilterModeDirectoryTree))
				if ((pSearchResult->m_LastError=LFGetDefaultStore(pFilter->StoreID))!=LFOk)
					goto Finish;

			QueryTree(pFilter, pSearchResult);

			break;

		case LFFilterModeSearch:
			QuerySearch(pFilter, pSearchResult);

			break;

		default:
			pSearchResult->m_LastError = LFIllegalQuery;
		}
	}

Finish:
	pSearchResult->FinishQuery(pFilter);
	pSearchResult->m_QueryTime = GetTickCount()-Start;

	return pSearchResult;
}

LFCORE_API LFSearchResult* LFQueryEx(LFFilter* pFilter, LFSearchResult* pSearchResult, INT First, INT Last)
{
	DWORD Start = GetTickCount();

	if ((pFilter->Mode>=LFFilterModeDirectoryTree) && (pFilter->Options.IsSubfolder) && (pSearchResult->m_RawCopy) &&
		(First<=Last) && (First>=0) && (First<(INT)pSearchResult->m_ItemCount) && (Last>=0) && (Last<(INT)pSearchResult->m_ItemCount))
	{
		pSearchResult->m_LastError = LFOk;
		pSearchResult->KeepRange(First, Last);
		pSearchResult->FinishQuery(pFilter);
	}
	else
	{
		pSearchResult = new LFSearchResult(LFContextSubfolderDefault);
		pSearchResult->m_LastError = LFIllegalQuery;
	}

	pSearchResult->m_QueryTime = GetTickCount()-Start;

	return pSearchResult;
}
