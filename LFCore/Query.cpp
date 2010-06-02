#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "Query.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <string>


#include "ShellProperties.h"


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;


int PassesFilterCore(LFCoreAttributes* ca, LFFilter* filter)
{
	assert(filter);
	assert(ca);

	// StoreID wird durch Query Optimization bearbeitet

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

	// TODO
	return 0;
}

bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* filter)
{
	assert(filter);
	assert(i);

	// TODO
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

	if (filter->Options.AddBacklink)
	{
		LFFilter* nf = LFAllocFilter();
		nf->Mode = LFFilterModeStores;
		nf->Options = filter->Options;

		res->AddBacklink("", nf);
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
					if (a>=LFFirstSoloDomain)
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

LFSearchResult* QueryStore(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextDefault);
	res->m_RecommendedView = LFViewDetails;
	res->m_LastError = LFOk;

	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	res->m_LastError = OpenStore(&filter->StoreID[0], false, idx1, idx2, &slot, &StoreLock);
	if (res->m_LastError==LFOk)
	{
		if ((filter->Options.AddBacklink) && (filter->Mode==LFFilterModeDirectoryTree))
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

		if (filter->Mode==LFFilterModeDirectoryTree)
		{
			switch (filter->DomainID)
			{
			case LFDomainTrash:
				res->m_Context = LFContextHousekeeping;
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
			filter->Result.FilterType = LFFilterTypeQueryFilter;
		}
	}
	else
	{
		filter->Result.FilterType = LFFilterTypeError;
	}

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
		if ((strcmp(filter->StoreID, "")==0) && (filter->Mode>=LFFilterModeStoreHome) && (filter->Mode<=LFFilterModeSearchInStore))
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
			case LFFilterModeSearchInStore:
				res = QueryStore(filter);
				break;
			default:
				res = new LFSearchResult(LFContextDefault);
				res->m_LastError = LFIllegalQuery;
				filter->Result.FilterType = LFFilterTypeIllegalRequest;
			}

		// Statistik
		if ((wcscmp(filter->Name, L"")==0) && (res->m_Context!=LFContextDefault))
			LoadString(LFCoreModuleHandle, res->m_Context+IDS_FirstContext, filter->Name, 256);
		GetLocalTime(&filter->Result.Time);
		filter->Result.ItemCount = res->m_ItemCount;
		filter->Result.FileCount = res->m_FileCount;
		filter->Result.FileSize = res->m_FileSize;
	}

	res->m_QueryTime = GetTickCount()-start;
	return res;
}
