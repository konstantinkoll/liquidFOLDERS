#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "Query.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <string>

extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;


LFCore_API bool LFPassesFilter(LFItemDescriptor* i, LFFilter* filter)
{
	// TODO

	return true;
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
		unsigned int cnt[LFDomainCount];
		idx1->RetrieveDomains(cnt);
		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);

		for (unsigned int a=0; a<LFDomainCount; a++)
			if ((cnt[a]) || (!filter->HideEmptyDomains))
			{
				LFDomainDescriptor* d = LFGetDomainInfo(a);
				char FileID[LFKeySize];
				sprintf_s(FileID, LFKeySize, "%d", a);
				wchar_t Hint[256];
				swprintf_s(Hint, 256, cnt[a]==1 ? HintSingular : HintPlural, cnt[a]);

				LFFilter* nf = LFAllocFilter();
				nf->Mode = LFFilterModeDirectoryTree;
				nf->Options = filter->Options;
				strcpy_s(nf->StoreID, LFKeySize, filter->StoreID);
				wcscpy_s(nf->Name, 256, d->DomainName);

				res->AddItemDescriptor(AllocFolderDescriptor(d->DomainName, d->Comment, Hint, filter->StoreID, FileID, d->IconID, d->CategoryID, nf));
				LFFreeDomainDescriptor(d);
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
	LFStoreDescriptor s;
	HANDLE StoreLock = NULL;
	res->m_LastError = OpenStore(&filter->StoreID[0], false, idx1, idx2, &s, &StoreLock);
	if (res->m_LastError==LFOk)
	{
		if ((filter->Options.AddBacklink) && (filter->Mode==LFFilterModeDirectoryTree))
		{
			LFFilter* nf = LFAllocFilter();
			nf->Mode = LFFilterModeStoreHome;
			nf->Options = filter->Options;
			strcpy_s(nf->StoreID, LFKeySize, s.StoreID);
			wcscpy_s(nf->Name, 256, s.StoreName);

			res->AddBacklink(filter->StoreID, nf);
		}

		idx1->Retrieve(filter, res);
		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);

		filter->Result.FilterType = (filter->Mode==LFFilterModeDirectoryTree) ? LFFilterTypeSubfolder : LFFilterTypeQueryFilter;
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
		filter->Result.ItemCount = res->m_Count;
	}

	res->m_QueryTime = GetTickCount()-start;
	return res;
}
