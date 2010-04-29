#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Domains.h"
#include "Mutex.h"
#include "StoreCache.h"
#include <string>

extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;


LFItemDescriptor* CreateDomainItem(const wchar_t* Name, const wchar_t* Hint, const char* StoreID, const char* FileID, unsigned int IconID, unsigned int CategoryID, LFFilter* nf)
{
	LFItemDescriptor* d = LFAllocItemDescriptor();

	d->IconID = IconID;
	d->CategoryID = CategoryID;
	d->Type = LFTypeVirtual;
	SetAttribute(d, LFAttrFileName, Name);
	SetAttribute(d, LFAttrStoreID, StoreID);
	SetAttribute(d, LFAttrFileID, FileID);
	SetAttribute(d, LFAttrHint, Hint);

	if (!nf)
	{
		nf = LFAllocFilter();
		nf->Mode = LFFilterModeSearchInStore;
		nf->AllowSubfolders = true;
		strcpy_s(nf->StoreID, LFKeySize, StoreID);
		wcscpy_s(nf->Name, 256, Name);
	}
	d->NextFilter = nf;

	return d;
}

LFSearchResult* QueryDomains(LFFilter* f)
{
	LFSearchResult* res = new LFSearchResult(LFContextStoreHome);
	res->m_RecommendedView = LFViewSmallIcons;
	res->m_HasCategories = true;

	if (!f->Legacy)
	{
		LFFilter* nf = LFAllocFilter();
		nf->Mode = LFFilterModeStores;
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
			res->AddItemDescriptor(CreateDomainItem(d->DomainName, d->Hint, f->StoreID, FileID, d->IconID, d->CategoryID, NULL));
			LFFreeDomainDescriptor(d);
		}

		ReleaseMutexForStore(StoreLock);
	}
	else
	{
		res->m_LastError = LFIllegalKey;
	}

	if (!f->Legacy)
	{
		//TODO: Filter
	}

	f->Result.FilterType = LFFilterTypeStoreHome;
	return res;
}
