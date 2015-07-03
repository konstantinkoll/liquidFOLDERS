
#include "stdafx.h"
#include "Categorizer.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "Query.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <shlwapi.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern const INT CoreOffsets[];
extern const BYTE AttrTypes[];


BOOL CheckCondition(void* value, LFFilterCondition* c)
{
	assert(c->Compare>=LFFilterCompareIgnore);
	assert(c->Compare<=LFFilterCompareContains);

	switch (c->Compare)
	{
	case LFFilterCompareIgnore:
		return TRUE;
	case LFFilterCompareIsNull:
		return IsNullValue(AttrTypes[c->AttrData.Attr], value);
	}

	if (!value)
		switch(c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return LFIsNullVariantData(c->AttrData);
		case LFFilterCompareIsNotEqual:
			return !LFIsNullVariantData(c->AttrData);
		default:
			return FALSE;
		}

	assert(c->AttrData.Attr<LFAttributeCount);
	assert(c->AttrData.Type==AttrTypes[c->AttrData.Attr]);
	assert(c->AttrData.Type<LFTypeCount);

	ULARGE_INTEGER uli1;
	ULARGE_INTEGER uli2;
	size_t len1;
	size_t len2;
	FILETIME ft;
	WCHAR* conditionarray;
	WCHAR condition[256];
	WCHAR* tagarray;
	WCHAR tag[256];
	CHAR Server[256];

	switch (c->AttrData.Type)
	{
	case LFTypeUnicodeString:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			if (c->AttrData.Attr==LFAttrFileName)
			{
				len1 = wcslen((WCHAR*)value);
				len2 = wcslen(c->AttrData.UnicodeString);
				if (len1<=len2)
					return FALSE;
				return _wcsnicmp((WCHAR*)value, c->AttrData.UnicodeString, len2)==0;
			}
		case LFFilterCompareIsEqual:
			return _wcsicmp((WCHAR*)value, c->AttrData.UnicodeString)==0;
		case LFFilterCompareIsNotEqual:
			return _wcsicmp((WCHAR*)value, c->AttrData.UnicodeString)!=0;
		case LFFilterCompareBeginsWith:
			len1 = wcslen(c->AttrData.UnicodeString);
			len2 = wcslen((WCHAR*)value);
			if (len1>len2)
				return FALSE;
			return _wcsnicmp((WCHAR*)value, c->AttrData.UnicodeString, len1)==0;
		case LFFilterCompareEndsWith:
			len1 = wcslen(c->AttrData.UnicodeString);
			len2 = wcslen((WCHAR*)value);
			if (len1>len2)
				return FALSE;
			return _wcsicmp(&((WCHAR*)value)[len2-len1], c->AttrData.UnicodeString)==0;
		case LFFilterCompareContains:
			return StrStrIW((WCHAR*)value, c->AttrData.UnicodeString)!=NULL;
		default:
			
			assert(FALSE);
			return FALSE;
		}
	case LFTypeUnicodeArray:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			tagarray = (WCHAR*)value;
			while (GetNextTag(&tagarray, tag, 256))
				if (_wcsicmp(tag, c->AttrData.UnicodeArray)==0)
					return TRUE;
			return FALSE;
		case LFFilterCompareContains:
			conditionarray = c->AttrData.UnicodeArray;
			while (GetNextTag(&conditionarray, condition, 256))
			{
				tagarray = (WCHAR*)value;
				while (GetNextTag(&tagarray, tag, 256))
					if (_wcsicmp(tag, condition)==0)
						return TRUE;
			}
			return FALSE;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeAnsiString:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			if (c->AttrData.Attr==LFAttrURL)
			{
				CURLCategorizer::GetServer((CHAR*)value, Server, 256);
				return _stricmp(Server, c->AttrData.AnsiString)==0;
			}
		case LFFilterCompareIsEqual:
			return _stricmp((CHAR*)value, c->AttrData.AnsiString)==0;
		case LFFilterCompareIsNotEqual:
			return _stricmp((CHAR*)value, c->AttrData.AnsiString)!=0;
		case LFFilterCompareBeginsWith:
			len1 = strlen(c->AttrData.AnsiString);
			len2 = strlen((CHAR*)value);
			if (len1>len2)
				return FALSE;
			return _strnicmp((CHAR*)value, c->AttrData.AnsiString, len1)==0;
		case LFFilterCompareEndsWith:
			len1 = strlen(c->AttrData.AnsiString);
			len2 = strlen((CHAR*)value);
			if (len1>len2)
				return FALSE;
			return _stricmp(&((CHAR*)value)[len2-len1], c->AttrData.AnsiString)==0;
		case LFFilterCompareContains:
			return StrStrIA((CHAR*)value, c->AttrData.AnsiString)!=NULL;
		default:
			
			assert(FALSE);
			return FALSE;
		}
	case LFTypeFourCC:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(UINT*)value==c->AttrData.UINT32;
		case LFFilterCompareIsNotEqual:
			return *(UINT*)value!=c->AttrData.UINT32;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeRating:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			return CRatingCategorizer::GetRatingCategory(*(BYTE*)value)==CRatingCategorizer::GetRatingCategory(c->AttrData.Rating);
		case LFFilterCompareIsEqual:
			return *(BYTE*)value==c->AttrData.Rating;
		case LFFilterCompareIsNotEqual:
			return *(BYTE*)value!=c->AttrData.Rating;
		case LFFilterCompareIsAboveOrEqual:
			return *(BYTE*)value>=c->AttrData.Rating;
		case LFFilterCompareIsBelowOrEqual:
			return *(BYTE*)value<=c->AttrData.Rating;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeUINT:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(UINT*)value==c->AttrData.UINT32;
		case LFFilterCompareIsNotEqual:
			return *(UINT*)value!=c->AttrData.UINT32;
		case LFFilterCompareIsAboveOrEqual:
			return *(UINT*)value>=c->AttrData.UINT32;
		case LFFilterCompareIsBelowOrEqual:
			return *(UINT*)value<=c->AttrData.UINT32;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeSize:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			if (c->AttrData.Attr==LFAttrFileSize)
				return CSizeCategorizer::GetSizeCategory(*(INT64*)value)==CSizeCategorizer::GetSizeCategory(c->AttrData.INT64);
		case LFFilterCompareIsEqual:
			return *(INT64*)value==c->AttrData.INT64;
		case LFFilterCompareIsNotEqual:
			return *(INT64*)value!=c->AttrData.INT64;
		case LFFilterCompareIsAboveOrEqual:
			return *(INT64*)value>=c->AttrData.INT64;
		case LFFilterCompareIsBelowOrEqual:
			return *(INT64*)value<=c->AttrData.INT64;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeFraction:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.Fraction, sizeof(LFFraction))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.Fraction, sizeof(LFFraction))!=0;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeDouble:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return *(DOUBLE*)value==c->AttrData.Double;
		case LFFilterCompareIsNotEqual:
			return *(DOUBLE*)value!=c->AttrData.Double;
		case LFFilterCompareIsAboveOrEqual:
			return *(DOUBLE*)value>=c->AttrData.Double;
		case LFFilterCompareIsBelowOrEqual:
			return *(DOUBLE*)value<=c->AttrData.Double;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeFlags:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return (*(UINT*)value & c->AttrData.Flags.Mask)==(c->AttrData.Flags.Flags & c->AttrData.Flags.Mask);
		case LFFilterCompareIsNotEqual:
			return (*(UINT*)value & c->AttrData.Flags.Mask)!=(c->AttrData.Flags.Flags & c->AttrData.Flags.Mask);
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeGeoCoordinates:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))!=0;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeTime:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			CDateCategorizer::GetDay((FILETIME*)value, &ft);
			return memcmp(&ft, &c->AttrData.Time, sizeof(FILETIME))==0;
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.Time, sizeof(FILETIME))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.Time, sizeof(FILETIME))!=0;
		case LFFilterCompareIsAboveOrEqual:
			uli1.LowPart = (*(FILETIME*)value).dwLowDateTime;
			uli1.HighPart = (*(FILETIME*)value).dwHighDateTime;
			uli2.LowPart = c->AttrData.Time.dwLowDateTime;
			uli2.HighPart = c->AttrData.Time.dwHighDateTime;
			return uli1.QuadPart>=uli2.QuadPart;
		case LFFilterCompareIsBelowOrEqual:
			uli1.LowPart = (*(FILETIME*)value).dwLowDateTime;
			uli1.HighPart = (*(FILETIME*)value).dwHighDateTime;
			uli2.LowPart = c->AttrData.Time.dwLowDateTime;
			uli2.HighPart = c->AttrData.Time.dwHighDateTime;
			return uli1.QuadPart<=uli2.QuadPart;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeDuration:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
			return CDurationCategorizer::GetDurationCategory(*(UINT*)value)==CDurationCategorizer::GetDurationCategory(c->AttrData.UINT32);
		case LFFilterCompareIsEqual:
			return *(UINT*)value==c->AttrData.UINT32;
		case LFFilterCompareIsNotEqual:
			return *(UINT*)value!=c->AttrData.UINT32;
		case LFFilterCompareIsAboveOrEqual:
			return *(UINT*)value>=c->AttrData.UINT32;
		case LFFilterCompareIsBelowOrEqual:
			return *(UINT*)value<=c->AttrData.UINT32;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeBitrate:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return ((*(UINT*)value+500)/1000)==((c->AttrData.UINT32+500)/1000);
		case LFFilterCompareIsNotEqual:
			return ((*(UINT*)value+500)/1000)!=((c->AttrData.UINT32+500)/1000);
		case LFFilterCompareIsAboveOrEqual:
			return *(UINT*)value>=c->AttrData.UINT32;
		case LFFilterCompareIsBelowOrEqual:
			return *(UINT*)value<=c->AttrData.UINT32;
		default:
			assert(FALSE);
			return FALSE;
		}
	case LFTypeMegapixel:
		switch (c->Compare)
		{
		case LFFilterCompareSubfolder:
		case LFFilterCompareIsEqual:
			return (UINT)*((DOUBLE*)value)==(UINT)c->AttrData.Megapixel;
		case LFFilterCompareIsNotEqual:
			return (UINT)*((DOUBLE*)value)!=(UINT)c->AttrData.Megapixel;
		case LFFilterCompareIsAboveOrEqual:
			return *(DOUBLE*)value>=c->AttrData.Megapixel;
		case LFFilterCompareIsBelowOrEqual:
			return *(DOUBLE*)value<=c->AttrData.Megapixel;
		default:
			assert(FALSE);
			return FALSE;
		}
	}

	// Something fishy is going on - play it safe and include the file
	assert(FALSE);
	return TRUE;
}

INT PassesFilterCore(LFCoreAttributes* ca, LFFilter* f)
{
	assert(ca);
	assert(f);

	// Contexts
	if (ca->Flags & LFFlagTrash)
	{
		if (f->ContextID!=LFContextTrash)
			return -1;
	}
	else
		if (ca->Flags & LFFlagArchive)
		{
			if (f->ContextID!=LFContextArchive)
				return -1;
		}

	if ((f->ContextID) || (ca->ContextID==LFContextFilters))
		switch (f->ContextID)
		{
		case LFContextFavorites:
			if (!ca->Rating)
				return -1;
			break;
		case LFContextNew:
			if (!(ca->Flags & LFFlagNew))
				return -1;
			break;
		case LFContextArchive:
			if (!(ca->Flags & LFFlagArchive))
				return -1;
			break;
		case LFContextTrash:
			if (!(ca->Flags & LFFlagTrash))
				return -1;
			break;
		case LFContextFilters:
			if (f->Options.IsPersistent)
				return -1;
		default:
			if (f->ContextID!=ca->ContextID)
				return -1;
		}

	// Attributes
	INT advanced = (f->Searchterm[0]==L'\0') ? 1 : 0;

	LFFilterCondition* c = f->ConditionList;
	while (c)
	{
		if (c->AttrData.Attr<=LFLastCoreAttribute)
		{
			if (CoreOffsets[c->AttrData.Attr]!=-1)
				if (!CheckCondition((CHAR*)ca+CoreOffsets[c->AttrData.Attr], c))
					return -1;
		}
		else
		{
			advanced = 0;
		}

		c = c->Next;
	}

	return advanced;
}

BOOL PassesFilterSlaves(LFItemDescriptor* i, LFFilter* f)
{
	assert(f);
	assert(i);

	// Attributes
	LFFilterCondition* c = f->ConditionList;
	while (c)
	{
		if (c->AttrData.Attr>LFLastCoreAttribute)
			if (!CheckCondition(i->AttributeValues[c->AttrData.Attr], c))
				return FALSE;

		c = c->Next;
	}

	if (f->Searchterm[0]=='\0')
		return TRUE;

	// Globaler Suchbegriff
	BOOL checkTime = TRUE;
	for (UINT a=0; a<wcslen(f->Searchterm); a++)
	{
		WCHAR ch = f->Searchterm[a];
		if (((ch<L'0') || (ch>L'9')) && (ch!=L':') && (ch!=L'.') && (ch!=L'/') && (ch!=L'.'))
		{
			checkTime = FALSE;
			break;
		}
	}

	CHAR Searchterm[256];
	WideCharToMultiByte(CP_ACP, 0, f->Searchterm, -1, Searchterm, 256, NULL, NULL);

	for (UINT a=0; a<LFAttributeCount; a++)
		if ((a!=LFAttrStoreID) && (a!=LFAttrFileID) && (i->AttributeValues[a]))
		{
			WCHAR tmpStr[256];

			switch (AttrTypes[a])
			{
			case LFTypeUnicodeString:
			case LFTypeUnicodeArray:
				if (StrStrIW((WCHAR*)i->AttributeValues[a], f->Searchterm)!=NULL)
					return TRUE;
				break;
			case LFTypeAnsiString:
				if (StrStrIA((CHAR*)i->AttributeValues[a], Searchterm)!=NULL)
					return TRUE;
				break;
			case LFTypeGeoCoordinates:
				break;
			case LFTypeTime:
				if (!checkTime)
					break;
			default:
				LFAttributeToString(i, a, tmpStr, 256);
				if (StrStrIW(tmpStr, f->Searchterm)!=NULL)
					return TRUE;
			}
		}

	return FALSE;
}


// Query helpers

void RetrieveStore(CHAR* StoreID, LFFilter* f, LFSearchResult* sr)
{
	OPEN_STORE(StoreID, FALSE, sr->m_LastError = Result);

	if (idx1)
		idx1->Retrieve(f, sr);

	CLOSE_STORE();
}

void QueryStores(LFFilter* f, LFSearchResult* sr)
{
	sr->m_HasCategories = TRUE;

	// Volumes
	if (f)
		if (f->Options.AddVolumes)
			sr->AddVolumes();

	// Stores
	if (GetMutex(Mutex_Stores))
	{
		AddStoresToSearchResult(sr);
		ReleaseMutex(Mutex_Stores);
	}
	else
	{
		sr->m_LastError = LFMutexError;
	}
}

__forceinline void QueryTree(LFFilter* f, LFSearchResult* sr)
{
	RetrieveStore(f->StoreID, f, sr);
}

__forceinline void QuerySearch(LFFilter* f, LFSearchResult* sr)
{
	if (f->StoreID[0]=='\0')
	{
		// All stores
		if (!GetMutex(Mutex_Stores))
		{
			sr->m_LastError = LFMutexError;
			return;
		}

		CHAR* IDs;
		UINT count = FindStores(&IDs);
		ReleaseMutex(Mutex_Stores);

		if (count)
		{
			CHAR* ptr = IDs;
			for (UINT a=0; a<count; a++)
			{
				RetrieveStore(ptr, f, sr);
				ptr += LFKeySize;
			}

			free(IDs);
		}
	}
	else
	{
		// Single store
		RetrieveStore(f->StoreID, f, sr);
	}
}


// Public functions

LFCORE_API LFSearchResult* LFQuery(LFFilter* f)
{
	DWORD start = GetTickCount();

	LFSearchResult* sr = new LFSearchResult(f);

	if (!f)
	{
		QueryStores(f, sr);
	}
	else
	{
		// Ggf. Default Store einsetzen
		if ((f->StoreID[0]=='\0') && (f->Mode==LFFilterModeDirectoryTree))
			if (LFDefaultStoreAvailable())
			{
				LFGetDefaultStore(f->StoreID);
			}
			else
			{
				sr->m_LastError = LFNoDefaultStore;
				goto Finish;
			}

		// Query
		switch (f->Mode)
		{
		case LFFilterModeStores:
			QueryStores(f, sr);
			break;
		case LFFilterModeDirectoryTree:
			QueryTree(f, sr);
			break;
		case LFFilterModeSearch:
			QuerySearch(f, sr);
			break;
		default:
			sr->m_LastError = LFIllegalQuery;
		}
	}

Finish:
	sr->m_QueryTime = GetTickCount()-start;

	return sr;
}

LFCORE_API LFSearchResult* LFQuery(LFFilter* f, LFSearchResult* base, INT first, INT last)
{
	DWORD start = GetTickCount();

	LFSearchResult* sr;

	if ((f->Mode>=LFFilterModeDirectoryTree) && (f->Options.IsSubfolder) && (base->m_RawCopy) &&
		(first<=last) && (first>=0) && (first<(INT)base->m_ItemCount) && (last>=0) && (last<(INT)base->m_ItemCount))
	{
		sr = base;

		sr->m_LastError = LFOk;
		sr->KeepRange(first, last);
		sr->SetMetadataFromFilter(f);
	}
	else
	{
		sr = new LFSearchResult(LFContextSubfolderDefault);
		sr->m_LastError = LFIllegalQuery;
	}

	sr->m_QueryTime = GetTickCount()-start;

	return sr;
}

LFCORE_API LFStatistics* LFQueryStatistics(CHAR* StoreID)
{
	LFStatistics* stat = new LFStatistics();
	ZeroMemory(stat, sizeof(LFStatistics));

	if (!StoreID)
	{
		stat->LastError = LFIllegalKey;
		return stat;
	}

	if (!GetMutex(Mutex_Stores))
	{
		stat->LastError = LFMutexError;
		return stat;
	}

	// All stores
	CHAR* IDs;
	UINT count = FindStores(&IDs);
	ReleaseMutex(Mutex_Stores);

	if (count)
	{
		CHAR* ptr = IDs;
		for (UINT a=0; a<count; a++)
		{
			if ((*StoreID=='\0') || (strcmp(ptr, StoreID)==0))
			{
				HANDLE StoreLock = NULL;
				LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);

				if (slot)
				{
					for (UINT b=0; b<=min(LFLastQueryContext, 31); b++)
					{
						stat->FileCount[b] += slot->FileCount[b];
						stat->FileSize[b] += slot->FileSize[b];
					}

					ReleaseMutexForStore(StoreLock);
				}
			}

			ptr += LFKeySize;
		}

		free(IDs);
	}

	return stat;
}