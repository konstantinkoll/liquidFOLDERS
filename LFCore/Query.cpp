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
			res->AddDrives();

		filter->Result.FilterType = LFFilterTypeStores;
	}

	return res;
}






LFItemDescriptor* CreateDomainItem(const wchar_t* Name, const wchar_t* Hint, const char* StoreID, const char* FileID, unsigned int IconID, unsigned int CategoryID, LFFilter* Filter)
{
	LFItemDescriptor* d = LFAllocItemDescriptor();

	d->IconID = IconID;
	d->CategoryID = CategoryID;
	d->Type = LFTypeVirtual;
	d->NextFilter = Filter;

	SetAttribute(d, LFAttrFileName, Name);
	SetAttribute(d, LFAttrStoreID, StoreID);
	SetAttribute(d, LFAttrFileID, FileID);
	SetAttribute(d, LFAttrHint, Hint);

	return d;
}

LFSearchResult* QueryDomains(LFFilter* f)
{
	LFSearchResult* res = new LFSearchResult(LFContextStoreHome);
	res->m_RecommendedView = LFViewSmallIcons;
	res->m_HasCategories = true;

	if (f->Options.AddBacklink)
	{
		LFFilter* nf = LFAllocFilter();
		nf->Mode = LFFilterModeStores;
		nf->Options = f->Options;
		wchar_t BacklinkName[256];
		LoadString(LFCoreModuleHandle, IDS_BacklinkName, BacklinkName, 256);
		wchar_t BacklinkHint[256];
		LoadString(LFCoreModuleHandle, IDS_BacklinkHint, BacklinkHint, 256);
		res->AddItemDescriptor(CreateDomainItem(BacklinkName, BacklinkHint, f->StoreID, "BACK", IDI_FLD_Back, LFCategoryStore, nf));
	}

	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(f->StoreID, &StoreLock);
	ReleaseMutex(Mutex_Stores);

	if (slot)
	{
		for (unsigned int a=0; a<LFDomainCount; a++)
		{
			LFDomainDescriptor* d = LFGetDomainInfo(a);
			char FileID[LFKeySize];
			sprintf_s(FileID, sizeof(FileID), "%d", a);

			LFFilter* nf = LFAllocFilter();
			nf->Mode = LFFilterModeSearchInStore;
			nf->Options = f->Options;
			strcpy_s(nf->StoreID, LFKeySize, f->StoreID);
			wcscpy_s(nf->Name, 256, d->DomainName);

			res->AddItemDescriptor(CreateDomainItem(d->DomainName, d->Hint, f->StoreID, FileID, d->IconID, d->CategoryID, nf));
			LFFreeDomainDescriptor(d);
		}

		ReleaseMutexForStore(StoreLock);
	}
	else
	{
		res->m_LastError = LFIllegalKey;
	}

	f->Result.FilterType = LFFilterTypeStoreHome;
	return res;
}




LFSearchResult* QueryStore(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextDefault);
	res->m_RecommendedView = LFViewDetails;
	res->m_LastError = LFOk;

	CIndex* idx1 = NULL;
	CIndex* idx2 = NULL;
	HANDLE StoreLock = NULL;
	res->m_LastError = OpenStore(&filter->StoreID[0], false, idx1, idx2, &StoreLock);
	if (res->m_LastError==LFOk)
	{
		idx1->Retrieve(filter, res);
		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);
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
		if ((strcmp(filter->StoreID, "")==0) &&
			((filter->Mode==LFFilterModeStoreHome) || (filter->Mode==LFFilterModeSearchInStore)))
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
			default:
				res = QueryStore(filter);
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
