
#include "stdafx.h"
#include "Categorizers.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Query.h"
#include "Stores.h"
#include "TableApplications.h"
#include "TableAttributes.h"
#include <shlwapi.h>


#pragma comment(lib, "shlwapi.lib")


BOOL CheckCondition(LPCVOID pValue, LFFilterCondition* pFilterCondition)
{
	assert(pFilterCondition);
	assert(pFilterCondition->Compare>=LFFilterCompareIgnore);
	assert(pFilterCondition->Compare<=LFFilterCompareContains);
	assert(pFilterCondition->VData.Attr<LFAttributeCount);
	assert(pFilterCondition->VData.Type==AttrProperties[pFilterCondition->VData.Attr].Type);
	assert(pFilterCondition->VData.Type<LFTypeCount);

	switch (pFilterCondition->Compare)
	{
	case LFFilterCompareIgnore:
		return TRUE;

	case LFFilterCompareIsNull:
		return IsNullValue(pFilterCondition->VData.Type, pValue);
	}

	if (!pValue)
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return LFIsNullVariantData(pFilterCondition->VData);

		case LFFilterCompareIsNotEqual:
			return !LFIsNullVariantData(pFilterCondition->VData);

		default:
			return FALSE;
		}

	SIZE_T Len1;
	SIZE_T Len2;
	CHAR Server[256];
	FILETIME Time1;
	FILETIME Time2;
	LPCWSTR pHashtagArray;
	WCHAR Hashtag[256];

	switch (pFilterCondition->VData.Type)
	{
	case LFTypeUnicodeString:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			if (pFilterCondition->VData.Attr==LFAttrFileName)
			{
				Len1 = wcslen((LPCWSTR)pValue);
				Len2 = wcslen(pFilterCondition->VData.UnicodeString);

				return (Len1<=Len2) ? FALSE : _wcsnicmp((LPCWSTR)pValue, pFilterCondition->VData.UnicodeString, Len2)==0;
			}

		case LFFilterCompareIsEqual:
			return _wcsicmp((LPCWSTR)pValue, pFilterCondition->VData.UnicodeString)==0;

		case LFFilterCompareIsNotEqual:
			return _wcsicmp((LPCWSTR)pValue, pFilterCondition->VData.UnicodeString)!=0;
		case LFFilterCompareBeginsWith:
			Len1 = wcslen(pFilterCondition->VData.UnicodeString);
			Len2 = wcslen((LPCWSTR)pValue);

			return (Len1>Len2) ? FALSE :_wcsnicmp((LPCWSTR)pValue, pFilterCondition->VData.UnicodeString, Len1)==0;

		case LFFilterCompareEndsWith:
			Len1 = wcslen(pFilterCondition->VData.UnicodeString);
			Len2 = wcslen((LPCWSTR)pValue);

			return (Len1>Len2) ? FALSE : _wcsicmp(&((LPCWSTR)pValue)[Len2-Len1], pFilterCondition->VData.UnicodeString)==0;

		case LFFilterCompareContains:
			return StrStrIW((LPCWSTR)pValue, pFilterCondition->VData.UnicodeString)!=NULL;

		default:
			return FALSE;
		}

	case LFTypeUnicodeArray:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			return LFContainsHashtag((LPCWSTR)pValue, pFilterCondition->VData.UnicodeString);

		case LFFilterCompareContains:
			pHashtagArray = pFilterCondition->VData.UnicodeArray;
			while (GetNextHashtag(&pHashtagArray, Hashtag, 256))
				if (LFContainsHashtag((LPCWSTR)pValue, Hashtag))
					return TRUE;

		default:
			return FALSE;
		}

	case LFTypeAnsiString:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			if (pFilterCondition->VData.Attr==LFAttrURL)
			{
				CURLCategorizer::GetServer((LPCSTR)pValue, Server, 256);

				return _stricmp(Server, pFilterCondition->VData.AnsiString)==0;
			}

		case LFFilterCompareIsEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->VData.AnsiString)==0;

		case LFFilterCompareIsNotEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->VData.AnsiString)!=0;

		case LFFilterCompareBeginsWith:
			Len1 = strlen(pFilterCondition->VData.AnsiString);
			Len2 = strlen((LPCSTR)pValue);

			return (Len1>Len2) ? FALSE : _strnicmp((LPCSTR)pValue, pFilterCondition->VData.AnsiString, Len1)==0;

		case LFFilterCompareEndsWith:
			Len1 = strlen(pFilterCondition->VData.AnsiString);
			Len2 = strlen((LPCSTR)pValue);

			return (Len1>Len2) ? FALSE : _stricmp(&((LPCSTR)pValue)[Len2-Len1], pFilterCondition->VData.AnsiString)==0;

		case LFFilterCompareContains:
			return StrStrIA((LPCSTR)pValue, pFilterCondition->VData.AnsiString)!=NULL;

		default:
			return FALSE;
		}

	case LFTypeIATACode:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->VData.IATACode)==0;

		case LFFilterCompareIsNotEqual:
			return _stricmp((LPCSTR)pValue, pFilterCondition->VData.IATACode)!=0;

		default:
			return FALSE;
		}

	case LFTypeFourCC:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *((UINT*)pValue)==pFilterCondition->VData.UINT32;

		case LFFilterCompareIsNotEqual:
			return *((UINT*)pValue)!=pFilterCondition->VData.UINT32;

		default:
			return FALSE;
		}

	case LFTypeRating:
	case LFTypeColor:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			return CRatingCategorizer::GetRatingCategory(*((BYTE*)pValue))==CRatingCategorizer::GetRatingCategory(pFilterCondition->VData.Rating);

		case LFFilterCompareIsEqual:
			return *((BYTE*)pValue)==pFilterCondition->VData.Rating;

		case LFFilterCompareIsNotEqual:
			return *((BYTE*)pValue)!=pFilterCondition->VData.Rating;

		case LFFilterCompareIsAboveOrEqual:
			return *((BYTE*)pValue)>=pFilterCondition->VData.Rating;

		case LFFilterCompareIsBelowOrEqual:
			return *((BYTE*)pValue)<=pFilterCondition->VData.Rating;

		default:
			return FALSE;
		}

	case LFTypeUINT:
	case LFTypeDuration:
	case LFTypeGenre:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			if (pFilterCondition->VData.Type==LFTypeDuration)
				return CDurationCategorizer::GetDurationCategory(*((UINT*)pValue))==CDurationCategorizer::GetDurationCategory(pFilterCondition->VData.UINT32);

		case LFFilterCompareIsEqual:
			return *((UINT*)pValue)==pFilterCondition->VData.UINT32;

		case LFFilterCompareIsNotEqual:
			return *((UINT*)pValue)!=pFilterCondition->VData.UINT32;

		case LFFilterCompareIsAboveOrEqual:
			return *((UINT*)pValue)>=pFilterCondition->VData.UINT32;

		case LFFilterCompareIsBelowOrEqual:
			return *((UINT*)pValue)<=pFilterCondition->VData.UINT32;

		default:
			return FALSE;
		}

	case LFTypeSize:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
			return CSizeCategorizer::GetSizeCategory(*((INT64*)pValue))==CSizeCategorizer::GetSizeCategory(pFilterCondition->VData.INT64);

		case LFFilterCompareIsEqual:
			return *((INT64*)pValue)==pFilterCondition->VData.INT64;

		case LFFilterCompareIsNotEqual:
			return *((INT64*)pValue)!=pFilterCondition->VData.INT64;

		case LFFilterCompareIsAboveOrEqual:
			return *((INT64*)pValue)>=pFilterCondition->VData.INT64;

		case LFFilterCompareIsBelowOrEqual:
			return *((INT64*)pValue)<=pFilterCondition->VData.INT64;

		default:
			return FALSE;
		}

	case LFTypeFraction:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(pValue, &pFilterCondition->VData.Fraction, sizeof(LFFraction))==0;

		case LFFilterCompareIsNotEqual:
			return memcmp(pValue, &pFilterCondition->VData.Fraction, sizeof(LFFraction))!=0;

		default:
			return FALSE;
		}

	case LFTypeDouble:
	case LFTypeMegapixel:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *((DOUBLE*)pValue)==pFilterCondition->VData.Double;

		case LFFilterCompareIsNotEqual:
			return *((DOUBLE*)pValue)!=pFilterCondition->VData.Double;

		case LFFilterCompareIsAboveOrEqual:
			return *((DOUBLE*)pValue)>=pFilterCondition->VData.Double;

		case LFFilterCompareIsBelowOrEqual:
			return *((DOUBLE*)pValue)<=pFilterCondition->VData.Double;

		default:
			return FALSE;
		}

	case LFTypeGeoCoordinates:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(pValue, &pFilterCondition->VData.GeoCoordinates, sizeof(LFGeoCoordinates))==0;

		case LFFilterCompareIsNotEqual:
			return memcmp(pValue, &pFilterCondition->VData.GeoCoordinates, sizeof(LFGeoCoordinates))!=0;

		default:
			return FALSE;
		}

	case LFTypeTime:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			CDateCategorizer::GetDay((LPFILETIME)pValue, &Time1);
			CDateCategorizer::GetDay(&pFilterCondition->VData.Time, &Time2);

			return memcmp(&Time1, &Time2, sizeof(FILETIME))==0;

		case LFFilterCompareIsNotEqual:
			CDateCategorizer::GetDay((LPFILETIME)pValue, &Time1);
			CDateCategorizer::GetDay(&pFilterCondition->VData.Time, &Time2);

			return memcmp(&Time1, &Time2, sizeof(FILETIME))!=0;

		case LFFilterCompareIsAboveOrEqual:
			return CompareFileTime((LPFILETIME)pValue, &pFilterCondition->VData.Time)>=0;

		case LFFilterCompareIsBelowOrEqual:
			return CompareFileTime((LPFILETIME)pValue, &pFilterCondition->VData.Time)<=0;

		default:
			return FALSE;
		}

	case LFTypeBitrate:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return ((*(UINT*)pValue+500)/1000)==((pFilterCondition->VData.UINT32+500)/1000);

		case LFFilterCompareIsNotEqual:
			return ((*(UINT*)pValue+500)/1000)!=((pFilterCondition->VData.UINT32+500)/1000);

		case LFFilterCompareIsAboveOrEqual:
			return *((UINT*)pValue)>=pFilterCondition->VData.UINT32;

		case LFFilterCompareIsBelowOrEqual:
			return *((UINT*)pValue)<=pFilterCondition->VData.UINT32;

		default:
			return FALSE;
		}

	case LFTypeApplication:
		switch (pFilterCondition->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *((BYTE*)pValue)==pFilterCondition->VData.Application;

		case LFFilterCompareIsNotEqual:
			return *((BYTE*)pValue)!=pFilterCondition->VData.Application;

		default:
			return FALSE;
		}
	}

	return TRUE;
}

BOOL PassesFilter(UINT TableID, LPCVOID pTableData, LFFilter* pFilter, BYTE& QueryState)
{
	assert(TableID>=0);
	assert(TableID<IDXTABLECOUNT);
	assert(pTableData);
	assert(pFilter);

	// Primary checks on core attributes only
	if (TableID==IDXTABLE_MASTER)
	{
		const LPCCOREATTRIBUTES pCoreAttributes = (LPCCOREATTRIBUTES)pTableData;

		// Only show trashed files when filter queries trashcan
		if ((pCoreAttributes->State & LFItemStateTrash) && (pFilter->Query.Context!=LFContextTrash))
			return FALSE;

		// Only show archived files when filter queries archive
		if ((pCoreAttributes->State & LFItemStateArchive) && (pFilter->Query.Context!=LFContextArchive) &&
			(((pCoreAttributes->State & LFItemStateTrash)==0) || (pFilter->Query.Context!=LFContextTrash)))
			return FALSE;

		if (((pFilter->Query.Context!=LFContextAllFiles) && (pFilter->Query.Context!=LFContextAuto)) || LFIsFilterFile(*pCoreAttributes))
			switch (pFilter->Query.Context)
			{
			case LFContextFavorites:
				if (!pCoreAttributes->Rating)
					return FALSE;

				break;

			case LFContextTasks:
				if (!(pCoreAttributes->State & LFItemStateTask))
					return FALSE;

				break;

			case LFContextNew:
				if (!(pCoreAttributes->State & LFItemStateNew))
					return FALSE;

				break;

			case LFContextArchive:
				if (!(pCoreAttributes->State & LFItemStateArchive))
					return FALSE;

				break;

			case LFContextTrash:
				if (!(pCoreAttributes->State & LFItemStateTrash))
					return FALSE;

				break;

			case LFContextFilters:
				if (pFilter->IsPersistent)
					return FALSE;

			default:
				if (pFilter->Query.Context!=LFGetUserContextID(*pCoreAttributes))
					return FALSE;
			}
	}

	// Check index table
	const IdxTable* pTable = &IndexTables[TableID];
	BYTE Passed = QUERYSTATE_PASSED_MASTER | QUERYSTATE_PASSED_SLAVE;

	LFFilterCondition* pFilterCondition = pFilter->Query.pConditionList;
	while (pFilterCondition)
	{
		BOOL AttributePresent = FALSE;
		const ATTRIBUTE Attr = pFilterCondition->VData.Attr;

		for (UINT a=0; a<pTable->cTableEntries; a++)
			if (pTable->pTableEntries[a].Attr==Attr)
			{
				// Attribute exists for type
				if (!CheckCondition((BYTE*)pTableData+pTable->pTableEntries[a].Offset, pFilterCondition))
					return FALSE;

				AttributePresent = TRUE;
				break;
			}

		// Attribute does not exist for type
		if (!AttributePresent)
			if ((Attr>LFLastCoreAttribute) && (Attr!=LFAttrDimension) && (Attr!=LFAttrAspectRatio))
			{
				if ((TableID!=IDXTABLE_MASTER) &&								// Only exit on slave attributes when scanning slave
					(pFilterCondition->Compare!=LFFilterCompareIsNotEqual))		// Missing attributes are always "not equal"
					return FALSE;
	
				Passed &= ~QUERYSTATE_PASSED_SLAVE;
			}

		pFilterCondition = pFilterCondition->pNext;
	}

	QueryState |= Passed;

	// Search term
	if ((QueryState & QUERYSTATE_PASSED_SEARCHTERM)==0)
		if (pFilter->Query.SearchTerm[0])
		{
			// Check if search term contains letters: if yes, do not compare attributes that contain letters for improved efficiency
			assert(QUERYSTATE_SEARCHTERM_UNKNOWN==0);

			if (!(QueryState & QUERYSTATE_SEARCHTERMMASK))
			{
				QueryState |= QUERYSTATE_SEARCHTERM_NOLETTERS;

				LPCWSTR pChar = pFilter->Query.SearchTerm;
				while (*pChar)
				{
					if (((*pChar>=L'A') && (*pChar<=L'Z')) || ((*pChar>=L'a') && (*pChar<=L'z')))
					{
						QueryState = (QueryState & ~QUERYSTATE_SEARCHTERMMASK) | QUERYSTATE_SEARCHTERM_LETTERS;
						break;
					}

					pChar++;
				}
			}

			// Compare attributes
			for (UINT a=0; a<pTable->cTableEntries; a++)
			{
				const ATTRIBUTE Attr = pTable->pTableEntries[a].Attr;

				if ((QueryState & QUERYSTATE_SEARCHTERM_NOLETTERS) || (TypeProperties[AttrProperties[Attr].Type].DataFlags & LFDataContainsLetters))
				{
					WCHAR tmpStr[256];
					ToString((BYTE*)pTableData+pTable->pTableEntries[a].Offset, AttrProperties[pTable->pTableEntries[a].Attr].Type, tmpStr, 256);

					if (StrStrIW(tmpStr, pFilter->Query.SearchTerm)!=NULL)
					{
						QueryState |= QUERYSTATE_PASSED_SEARCHTERM;
						break;
					}
				}
			}
		}
		else
		{
			QueryState |= QUERYSTATE_PASSED_SEARCHTERM;
		}

	return TRUE;
}

BOOL PassesFilter(LFItemDescriptor* pItemDescriptor, LFFilter* pFilter, BYTE& QueryState)
{
	assert(pItemDescriptor);
	assert(pFilter);

	LFFilterCondition* pFilterCondition = pFilter->Query.pConditionList;
	while (pFilterCondition)
	{
		switch (pFilterCondition->VData.Attr)
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

	return (QueryState & QUERYSTATE_PASSEDMASK)==QUERYSTATE_PASSEDMASK;
}


// Query helper
//

void QueryStore(const ABSOLUTESTOREID& StoreID, LFFilter* pFilter, LFSearchResult* pSearchResult)
{
	assert(pSearchResult);

	CStore* pStore;
	UINT Result = OpenStore(StoreID, pStore, FALSE);

	if (Result==LFOk)
	{
		pStore->Query(pFilter, pSearchResult);
		delete pStore;
	}
	else
	{
		pSearchResult->m_LastError = Result;
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
		switch (pFilter->Query.Mode)
		{
		case LFFilterModeStores:
			QueryStores(pSearchResult);

			break;

		case LFFilterModeDirectoryTree:
			// Resolve default store
			if ((pSearchResult->m_LastError=LFResolveStoreID(pFilter->Query.StoreID))!=LFOk)
				break;

			// Run query

		case LFFilterModeQuery:
			if (!LFIsDefaultStoreID(pFilter->Query.StoreID))
			{
				// Single store ? we know the store ID is absolute!
				QueryStore(MAKEABSOLUTESTOREID(pFilter->Query.StoreID), pFilter, pSearchResult);
			}
			else
			{
				LPCABSOLUTESTOREID lpcStoreIDs;
				UINT Count;
				if ((pSearchResult->m_LastError=LFGetAllStores(lpcStoreIDs, Count))==LFOk)
				{
					// Iterate stores
					for (UINT a=0; a<Count; a++)
						QueryStore(lpcStoreIDs[a], pFilter, pSearchResult);

					free(lpcStoreIDs);
				}
			}

			break;

		default:
			pSearchResult->m_LastError = LFIllegalQuery;
		}
	}

	pSearchResult->FinishQuery(pFilter);
	pSearchResult->m_QueryTime = GetTickCount()-Start;

	return pSearchResult;
}

LFCORE_API LFSearchResult* LFQueryEx(LFFilter* pFilter, LFSearchResult* pSearchResult, INT First, INT Last)
{
	DWORD Start = GetTickCount();

	if ((pFilter->Query.Mode>=LFFilterModeDirectoryTree) && pFilter->IsSubfolder && pSearchResult->m_RawCopy &&
		(First<=Last) && (First>=0) && (First<(INT)pSearchResult->m_ItemCount) && (Last>=0) && (Last<(INT)pSearchResult->m_ItemCount))
	{
		pSearchResult->m_LastError = LFOk;
		pSearchResult->KeepRange((UINT)First, (UINT)Last);
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
