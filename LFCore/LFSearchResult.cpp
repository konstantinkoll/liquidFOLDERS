#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "IdxTables.h"
#include "LFSearchResult.h"
#include "StoreCache.h"
#include <assert.h>
#include <malloc.h>
#include <shellapi.h>


extern HMODULE LFCoreModuleHandle;


LFSearchResult::LFSearchResult(int ctx)
{
	m_RawCopy = true;
	m_LastError = LFOk;
	m_Context = ctx;
	m_ContextView = ctx;
	m_RecommendedView = LFViewDetails;
	m_HasCategories = false;
	m_QueryTime = 0;
	m_Files = NULL;
	m_Count = 0;
	m_Allocated = 0;
}

LFSearchResult::LFSearchResult(int ctx, LFSearchResult* res)
{
	m_RawCopy = false;
	m_LastError = res->m_LastError;
	m_Context = res->m_Context;
	m_ContextView = ctx;
	m_RecommendedView = res->m_RecommendedView;
	m_HasCategories = res->m_HasCategories;
	m_QueryTime = res->m_QueryTime;
	m_Files = static_cast<LFItemDescriptor**>(_aligned_malloc(res->m_Count*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
	m_Allocated = res->m_Count;
	if (m_Files)
	{
		memcpy(m_Files, res->m_Files, res->m_Count*sizeof(LFItemDescriptor*));
		m_Count = res->m_Count;
		for (unsigned int a=0; a<res->m_Count; a++)
			m_Files[a]->RefCount++;
	}
	else
	{
		m_LastError = LFMemoryError;
		m_Count = 0;
		m_Allocated =0;
	}
}

LFSearchResult::~LFSearchResult()
{
	if (m_Files)
	{
		for (unsigned int a=0; a<m_Count; a++)
			LFFreeItemDescriptor(m_Files[a]);
		_aligned_free(m_Files);
	}
}

bool LFSearchResult::AddItemDescriptor(LFItemDescriptor* i)
{
	if (!m_Files)
	{
		m_Files = static_cast<LFItemDescriptor**>(_aligned_malloc(LFSR_FirstAlloc*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
		if (!m_Files)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated = LFSR_FirstAlloc;
	}
	
	if (m_Count==m_Allocated)
	{
		m_Files = static_cast<LFItemDescriptor**>(_aligned_realloc(m_Files, (m_Allocated+LFSR_SubsequentAlloc)*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
		if (!m_Files)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated += LFSR_SubsequentAlloc;
	}

	if (m_RawCopy)
		i->Position = m_Count;
	m_Files[m_Count++] = i;

	return true;
}

bool LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* s, LFFilter* f)
{
	LFFilter* nf = LFAllocFilter();
	nf->Mode = LFFilterModeStoreHome;
	if (f)
		nf->Options = f->Options;
	strcpy_s(nf->StoreID, LFKeySize, s->StoreID);
	wcscpy_s(nf->Name, 256, s->StoreName);

	LFItemDescriptor* d = LFAllocItemDescriptor();
	bool IsMounted = IsStoreMounted(s);

	if (strcmp(s->StoreID, DefaultStore)==0)
	{
		d->IconID = IDI_STORE_Default;
		d->Type |= LFTypeDefaultStore;
		wchar_t ds[256];
		LoadStringW(LFCoreModuleHandle, IDS_DefaultStore, ds, 256);
		SetAttribute(d, LFAttrHint, ds);
	}
	else
	{
		d->IconID = (s->StoreMode==LFStoreModeInternal ? IDI_STORE_Empty : IDI_Bag);
		if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
			if (wcscmp(s->LastSeen, L"")!=0)
			{
				wchar_t ls[256];
				LoadStringW(LFCoreModuleHandle, IsMounted ? IDS_SeenOn :IDS_LastSeen, ls, 256);
				wchar_t hint[256];
				wsprintf(hint, ls, s->LastSeen);
				SetAttribute(d, LFAttrHint, hint);
			}
	}

	if (!IsMounted)
	{
		d->Type |= LFTypeGhosted | LFTypeNotMounted;
	}
	else
		// TODO
		if ((s->IndexVersion<CurIdxVersion) /*|| (s->MaintenanceTime<)*/)
			d->Type |= LFTypeRequiresMaintenance;

	d->CategoryID = s->StoreMode;
	d->Type |= LFTypeStore;
	SetAttribute(d, LFAttrFileName, s->StoreName);
	SetAttribute(d, LFAttrComment, s->Comment);
	SetAttribute(d, LFAttrStoreID, s->StoreID);
	SetAttribute(d, LFAttrFileID, s->StoreID);
	SetAttribute(d, LFAttrCreationTime, &s->CreationTime);
	SetAttribute(d, LFAttrFileTime, &s->FileTime);
	d->NextFilter = nf;

	bool res = AddItemDescriptor(d);
	if (!res)
		LFFreeItemDescriptor(d);

	return res;
}

void LFSearchResult::AddDrives(LFFilter* filter)
{
	DWORD DrivesOnSystem = LFGetLogicalDrives(LFGLD_External);

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if (!(DrivesOnSystem & 1))
			continue;

		wchar_t szDriveRoot[] = L" :\\";
		szDriveRoot[0] = cDrive;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			if ((!sfi.dwAttributes) && (filter->HideEmptyDrives))
				continue;

			LFItemDescriptor* d = LFAllocItemDescriptor();
			d->Type = LFTypeDrive;
			if (sfi.dwAttributes)
			{
				d->IconID = IDI_DRV_Default;
			}
			else
			{
				d->IconID = IDI_DRV_Empty;
				d->Type |= LFTypeGhosted | LFTypeNotMounted;
			}
			d->CategoryID = LFCategoryDrives;
			SetAttribute(d, LFAttrFileName, sfi.szDisplayName);
			char key[] = " :";
			key[0] = cDrive;
			SetAttribute(d, LFAttrFileID, key);
			SetAttribute(d, LFAttrHint, sfi.szTypeName);

			if (!AddItemDescriptor(d))
				LFFreeItemDescriptor(d);
		}
	}
}

void LFSearchResult::AddBacklink(char* StoreID, LFFilter* f)
{
	wchar_t BacklinkName[256];
	LoadString(LFCoreModuleHandle, IDS_BacklinkName, BacklinkName, 256);
	wchar_t BacklinkHint[256];
	LoadString(LFCoreModuleHandle, IDS_BacklinkHint, BacklinkHint, 256);

	LFItemDescriptor* d = AllocFolderDescriptor(BacklinkName, BacklinkHint, StoreID, "BACK", IDI_FLD_Back, LFCategoryStore, f);
	if (!AddItemDescriptor(d))
		delete d;
}

void LFSearchResult::RemoveItemDescriptor(unsigned int idx)
{
	assert(idx<m_Count);

	LFFreeItemDescriptor(m_Files[idx]);

	if (idx<--m_Count)
	{
		m_Files[idx] = m_Files[m_Count];
		if (m_RawCopy)
			m_Files[idx]->Position = idx;
	}
}

void LFSearchResult::RemoveFlaggedItemDescriptors()
{
	unsigned int idx = 0;
	
	while (idx<m_Count)
	{
		if (m_Files[idx]->DeleteFlag)
		{
			RemoveItemDescriptor(idx);
		}
		else
		{
			idx++;
		}
	}
}
