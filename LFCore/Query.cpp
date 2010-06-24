#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "Query.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern int CoreOffsets[];
extern unsigned char AttrTypes[];


wchar_t* wcsistr(const wchar_t* String, const wchar_t* Pattern)
{
	for (wchar_t* start=(wchar_t*)String; *start!=L'\0'; start++)
	{
		for (; ((*start!=L'\0') && (toupper(*start)!=toupper(*Pattern))); start++)
		;

		if (*start==L'\0')
			return NULL;

		wchar_t* pptr = (wchar_t*)Pattern;
		wchar_t* sptr = start;

		while (toupper(*sptr)==toupper(*pptr))
		{
			sptr++;
			pptr++;

			if (*pptr==L'\0')
				return start;
			
		}
	}

	return NULL;
}

char* stristr(const char* String, const char* Pattern)
{
	for (char* start=(char*)String; *start!='\0'; start++)
	{
		for (; ((*start!='\0') && (toupper(*start)!=toupper(*Pattern))); start++)
		;

		if (*start=='\0')
			return NULL;

		char* pptr = (char*)Pattern;
		char* sptr = start;

		while (toupper(*sptr)==toupper(*pptr))
		{
			sptr++;
			pptr++;

			if (*pptr=='\0')
				return start;
			
		}
	}

	return NULL;
}

bool CheckCondition(void* value, LFFilterCondition* c)
{
	assert(c->Compare>=LFFilterCompareIgnore);
	assert(c->Compare<=LFFilterCompareContains);

	if (c->Compare==LFFilterCompareIgnore)
		return true;

	if (!value)
		switch(c->Compare)
		{
		case LFFilterCompareIsEqual:
			return LFIsNullVariantData(&c->AttrData);
		case LFFilterCompareIsNotEqual:
			return !LFIsNullVariantData(&c->AttrData);
		default:
			return false;
		}

	assert(c->AttrData.Attr<LFAttributeCount);
	assert(c->AttrData.Type==AttrTypes[c->AttrData.Attr]);
	assert(c->AttrData.Type<LFTypeCount);

	ULARGE_INTEGER uli1;
	ULARGE_INTEGER uli2;
	size_t len1;
	size_t len2;

	switch (c->AttrData.Type)
	{
	case LFTypeUnicodeString:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return _wcsicmp((wchar_t*)value, c->AttrData.UnicodeString)==0;
		case LFFilterCompareIsNotEqual:
			return _wcsicmp((wchar_t*)value, c->AttrData.UnicodeString)!=0;
		case LFFilterCompareBeginsWith:
			len1 = wcslen((wchar_t*)value);
			len2 = wcslen(c->AttrData.UnicodeString);
			if (len1>len2)
				return false;
			return _wcsnicmp((wchar_t*)value, c->AttrData.UnicodeString, len1)==0;
		case LFFilterCompareEndsWith:
			len1 = wcslen((wchar_t*)value);
			len2 = wcslen(c->AttrData.UnicodeString);
			if (len1>len2)
				return false;
			return _wcsicmp((wchar_t*)value, &c->AttrData.UnicodeString[len2-len1])==0;
		case LFFilterCompareContains:
			return wcsistr(c->AttrData.UnicodeString, (wchar_t*)value)!=NULL;
		default:
			
			assert(false);
			return false;
		}
	case LFTypeAnsiString:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return _stricmp((char*)value, c->AttrData.AnsiString)==0;
		case LFFilterCompareIsNotEqual:
			return _stricmp((char*)value, c->AttrData.AnsiString)!=0;
		case LFFilterCompareBeginsWith:
			len1 = strlen((char*)value);
			len2 = strlen(c->AttrData.AnsiString);
			if (len1>len2)
				return false;
			return _strnicmp((char*)value, c->AttrData.AnsiString, len1)==0;
		case LFFilterCompareEndsWith:
			len1 = strlen((char*)value);
			len2 = strlen(c->AttrData.AnsiString);
			if (len1>len2)
				return false;
			return _stricmp((char*)value, &c->AttrData.AnsiString[len2-len1])==0;
		case LFFilterCompareContains:
			return stristr(c->AttrData.AnsiString, (char*)value)!=NULL;
		default:
			
			assert(false);
			return false;
		}
	case LFTypeFourCC:
	case LFTypeUINT:
	case LFTypeDuration:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return *(unsigned int*)value==c->AttrData.UINT;
		case LFFilterCompareIsNotEqual:
			return *(unsigned int*)value!=c->AttrData.UINT;
		case LFFilterCompareIsAboveOrEqual:
			return *(unsigned int*)value>=c->AttrData.UINT;
		case LFFilterCompareIsBelowOrEqual:
			return *(unsigned int*)value<=c->AttrData.UINT;
		default:
			assert(false);
			return false;
		}
	case LFTypeFlags:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return (*(unsigned int*)value & c->AttrData.Flags.Mask)==(c->AttrData.Flags.Flags & c->AttrData.Flags.Mask);
		case LFFilterCompareIsNotEqual:
			return (*(unsigned int*)value & c->AttrData.Flags.Mask)!=(c->AttrData.Flags.Flags & c->AttrData.Flags.Mask);
		default:
			assert(false);
			return false;
		}
	case LFTypeRating:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return *(unsigned char*)value==c->AttrData.Rating;
		case LFFilterCompareIsNotEqual:
			return *(unsigned char*)value!=c->AttrData.Rating;
		case LFFilterCompareIsAboveOrEqual:
			return *(unsigned char*)value>=c->AttrData.Rating;
		case LFFilterCompareIsBelowOrEqual:
			return *(unsigned char*)value<=c->AttrData.Rating;
		default:
			assert(false);
			return false;
		}
	case LFTypeINT64:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return *(__int64*)value==c->AttrData.INT64;
		case LFFilterCompareIsNotEqual:
			return *(__int64*)value!=c->AttrData.INT64;
		case LFFilterCompareIsAboveOrEqual:
			return *(__int64*)value>=c->AttrData.INT64;
		case LFFilterCompareIsBelowOrEqual:
			return *(__int64*)value<=c->AttrData.INT64;
		default:
			assert(false);
			return false;
		}
	case LFTypeFraction:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.Fraction, sizeof(LFFraction))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.Fraction, sizeof(LFFraction))!=0;
		default:
			assert(false);
			return false;
		}
	case LFTypeDouble:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return *(__int64*)value==c->AttrData.Double;
		case LFFilterCompareIsNotEqual:
			return *(__int64*)value!=c->AttrData.Double;
		case LFFilterCompareIsAboveOrEqual:
			return *(__int64*)value>=c->AttrData.Double;
		case LFFilterCompareIsBelowOrEqual:
			return *(__int64*)value<=c->AttrData.Double;
		default:
			assert(false);
			return false;
		}
	case LFTypeGeoCoordinates:
		switch (c->Compare)
		{
		case LFFilterCompareIsEqual:
			return memcmp(value, &c->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))==0;
		case LFFilterCompareIsNotEqual:
			return memcmp(value, &c->AttrData.GeoCoordinates, sizeof(LFGeoCoordinates))!=0;
		default:
			assert(false);
			return false;
		}
	case LFTypeTime:
		switch (c->Compare)
		{
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
			assert(false);
			return false;
		}
	default:
		assert(false);
	}

	// Something fishy is going on - play it safe and include the file
	return true;
}

int PassesFilterCore(LFCoreAttributes* ca, LFFilter* filter)
{
	assert(filter);
	assert(ca);

	// Domains
	if (filter->DomainID!=LFDomainTrash)
		if (ca->Flags & LFFlagTrash)
			return -1;

	if (filter->DomainID)
		switch (filter->DomainID)
		{
		case LFDomainAllMediaFiles:
			if ((ca->DomainID<LFDomainAudio) || (ca->DomainID>LFDomainVideos))
				return -1;
			break;
		case LFDomainFavorites:
			if (!ca->Rating)
				return -1;
			break;
		case LFDomainTrash:
			if (!(ca->Flags & LFFlagTrash))
				return -1;
			break;
		case LFDomainUnknown:
			if ((ca->DomainID) && (ca->DomainID<LFDomainCount))
				return -1;
			break;
		case LFDomainPictures:
			if (ca->DomainID==LFDomainPhotos)
				break;
		default:
			if (filter->DomainID!=ca->DomainID)
				return -1;
		}

	// Attribute
	int advanced = (filter->Searchterm[0]==L'\0') ? 0 : 1;

	LFFilterCondition* c = filter->ConditionList;
	while (c)
	{
		if (c->AttrData.Attr<=LFLastCoreAttribute)
		{
			if (CoreOffsets[c->AttrData.Attr]!=-1)
				if (!CheckCondition((char*)ca+CoreOffsets[c->AttrData.Attr], c))
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

bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* filter)
{
	assert(filter);
	assert(i);

	// Attribute
	LFFilterCondition* c = filter->ConditionList;
	while (c)
	{
		if (c->AttrData.Attr>LFLastCoreAttribute)
			if (!CheckCondition(i->AttributeValues[c->AttrData.Attr], c))
				return false;

		c = c->Next;
	}

	// Globaler Suchbegriff
	if (filter->Searchterm[0]!='\0')
	{
		// TODO
	}

	return true;
}

LFCore_API bool LFPassesFilter(LFItemDescriptor* i, LFFilter* filter)
{
	switch (PassesFilterCore(&i->CoreAttributes, filter))
	{
	case -1:
		return false;
	case 1:
		return true;
	default:
		return PassesFilterSlaves(i, filter);
	}
}

LFSearchResult* QueryStores(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextStores);
	res->m_RecommendedView = LFViewLargeIcons;
	res->m_LastError = LFOk;
	res->m_HasCategories = true;

	if (!GetMutex(Mutex_Stores))
	{
		res->m_LastError = LFMutexError;
		return res;
	}

	AddStoresToSearchResult(res, filter);
	ReleaseMutex(Mutex_Stores);

	if (filter)
	{
		if (filter->Options.AddDrives)
			res->AddDrives(filter);

		filter->Result.FilterType = LFFilterTypeStores;
	}

	return res;
}

LFSearchResult* QueryDomains(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextStoreHome);
	res->m_RecommendedView = LFViewSmallIcons;
	res->m_HasCategories = true;
	strcpy_s(res->m_StoreID, LFKeySize, filter->StoreID);

	if (filter->Options.AddBacklink)
	{
		LFFilter* nf = LFAllocFilter();
		nf->Mode = LFFilterModeStores;
		nf->Options = filter->Options;

		res->AddBacklink(filter->StoreID, nf);
	}

	wchar_t HintSingular[256];
	LoadString(LFCoreModuleHandle, IDS_HintSingular, HintSingular, 256);
	wchar_t HintPlural[256];
	LoadString(LFCoreModuleHandle, IDS_HintPlural, HintPlural, 256);

	CIndex* idx1;
	CIndex* idx2;
	HANDLE StoreLock = NULL;
	res->m_LastError = OpenStore(&filter->StoreID[0], false, idx1, idx2, NULL, &StoreLock);
	if (res->m_LastError==LFOk)
	{
		unsigned int cnt[LFDomainCount] = { 0 };
		__int64 size[LFDomainCount] = { 0 };
		if (idx1)
		{
			idx1->RetrieveStats(cnt, size);
			delete idx1;
		}
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);

		for (unsigned char a=0; a<LFDomainCount; a++)
			if ((cnt[a]) || (!filter->HideEmptyDomains) || (filter->UnhideAll))
			{
				LFDomainDescriptor* d = LFGetDomainInfo(a);
				char FileID[LFKeySize];
				sprintf_s(FileID, LFKeySize, "%d", a);
				wchar_t Hint[256];
				swprintf_s(Hint, 256, cnt[a]==1 ? HintSingular : HintPlural, cnt[a]);

				LFFilter* nf = LFAllocFilter();
				nf->Mode = LFFilterModeDirectoryTree;
				nf->Options = filter->Options;
				nf->DomainID = a;
				strcpy_s(nf->StoreID, LFKeySize, filter->StoreID);
				wcscpy_s(nf->Name, 256, d->DomainName);

				if (res->AddItemDescriptor(AllocFolderDescriptor(d->DomainName, d->Comment, Hint, filter->StoreID, FileID, &size[a], d->IconID, d->CategoryID, nf)))
					if ((a>=LFFirstSoloDomain) && (a!=LFDomainPhotos))
					{
						res->m_FileCount += cnt[a];
						res->m_FileSize += size[a];
					}

				LFFreeDomainDescriptor(d);
			}
			else
			{
				res->m_HidingItems = true;
			}

		filter->Result.FilterType = LFFilterTypeStoreHome;
	}
	else
	{
		filter->Result.FilterType = LFFilterTypeError;
	}

	return res;
}

bool RetrieveStore(char* StoreID, LFFilter* filter, LFSearchResult* res, bool AddBacklink)
{
	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	res->m_LastError = OpenStore(StoreID, false, idx1, idx2, &slot, &StoreLock);
	if (res->m_LastError==LFOk)
	{
		if (AddBacklink)
		{
			LFFilter* nf = LFAllocFilter();
			nf->Mode = LFFilterModeStoreHome;
			nf->Options = filter->Options;
			strcpy_s(nf->StoreID, LFKeySize, slot->StoreID);
			wcscpy_s(nf->Name, 256, slot->StoreName);

			res->AddBacklink(filter->StoreID, nf);
		}

		if (idx1)
		{
			idx1->Retrieve(filter, res);
			delete idx1;
		}
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);

		return true;
	}

	return false;
}

LFSearchResult* QueryTree(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextDefault);
	res->m_RecommendedView = LFViewDetails;
	res->m_LastError = LFOk;
	strcpy_s(res->m_StoreID, LFKeySize, filter->StoreID);

	if (RetrieveStore(filter->StoreID, filter, res, filter->Options.AddBacklink))
	{
		switch (filter->DomainID)
		{
		case LFDomainTrash:
			res->m_Context = LFContextTrash;
			filter->Result.FilterType = LFFilterTypeTrash;
			break;
		case LFDomainUnknown:
			res->m_Context = LFContextHousekeeping;
			filter->Result.FilterType = LFFilterTypeUnknownFileFormats;
			break;
		default:
			filter->Result.FilterType = LFFilterTypeSubfolder;
		}
	}
	else
	{
		filter->Result.FilterType = LFFilterTypeError;
	}

	return res;
}

LFSearchResult* QuerySearch(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextDefault);
	res->m_RecommendedView = LFViewDetails;
	res->m_LastError = LFOk;
	strcpy_s(res->m_StoreID, LFKeySize, filter->StoreID);

	bool success = false;
	if (filter->StoreID[0]=='\0')
	{
		// Alle Stores
		if (!GetMutex(Mutex_Stores))
		{
			res->m_LastError = LFMutexError;
			goto Finish;
		}

		char* keys;
		unsigned int count = FindStores(&keys);
		ReleaseMutex(Mutex_Stores);

		if (count)
		{
			char* ptr = keys;
			for (unsigned int a=0; a<count; a++)
			{
				if (RetrieveStore(ptr, filter, res, false))
					success = true;

				ptr += LFKeySize;
			}
		}

		free(keys);
	}
	else
	{
		// Ein Store
		success = RetrieveStore(filter->StoreID, filter, res, false);
	}

Finish:
	filter->Result.FilterType = success ? LFFilterTypeQueryFilter : LFFilterTypeError;

	return res;
}

LFCore_API LFSearchResult* LFQuery(LFFilter* filter)
{
	LFSearchResult* res = NULL;
	DWORD start = GetTickCount();

	if (!filter)
	{
		res = QueryStores(filter);
	}
	else
	{
		// Ggf. Default Store einsetzen
		if ((filter->StoreID[0]=='\0') && (filter->Mode>=LFFilterModeStoreHome) && (filter->Mode<=LFFilterModeDirectoryTree))
			if (LFDefaultStoreAvailable())
			{
				char* ds = LFGetDefaultStore();
				strcpy_s(filter->StoreID, LFKeySize, ds);
				free(ds);
			}
			else
			{
				res = new LFSearchResult(LFContextDefault);
				res->m_LastError = LFNoDefaultStore;
			}

		// Query
		if (!res)
			switch (filter->Mode)
			{
			case LFFilterModeStores:
				res = QueryStores(filter);
				break;
			case LFFilterModeStoreHome:
				res = QueryDomains(filter);
				break;
			case LFFilterModeDirectoryTree:
				res = QueryTree(filter);
				break;
			case LFFilterModeSearch:
				res = QuerySearch(filter);
				break;
			default:
				res = new LFSearchResult(LFContextDefault);
				res->m_LastError = LFIllegalQuery;
				filter->Result.FilterType = LFFilterTypeIllegalRequest;
			}

		// Statistik
		if ((filter->Name[0]==L'\0') && (res->m_Context!=LFContextDefault))
			LoadString(LFCoreModuleHandle, res->m_Context+IDS_FirstContext, filter->Name, 256);

		GetLocalTime(&filter->Result.Time);
		filter->Result.ItemCount = res->m_ItemCount;
		filter->Result.FileCount = res->m_FileCount;
		filter->Result.FileSize = res->m_FileSize;
	}

	res->m_QueryTime = GetTickCount()-start;
	return res;
}
