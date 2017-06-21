
#include "stdafx.h"
#include "AttributeTables.h"
#include "CIndex.h"
#include "CStore.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "Query.h"
#include "ShellProperties.h"
#include "Stores.h"
#include <assert.h>


// Macros
//

#define COMPACT(Index) \
	if (!RequiredSpaceAvailable(m_IdxPath, m_pTable[Index]->GetRequiredFileSize())) \
		return LFNotEnoughFreeDiscSpace; \
	if (!m_pTable[Index]->Compact()) \
		return LFIndexRepairError;

#define COUNT_FILE(Ops) \
	if (PtrM->Flags & LFFlagTrash) \
	{ \
		Ops(LFContextTrash); \
	} \
	else \
		if (PtrM->Flags & LFFlagArchive) \
		{ \
			Ops(LFContextArchive); \
		} \
		else \
		{ \
			if (PtrM->ContextID!=LFContextFilters) \
				Ops(LFContextAllFiles); \
			if (PtrM->ContextID!=LFContextAllFiles) \
				Ops(PtrM->ContextID); \
			if (PtrM->Rating) \
				Ops(LFContextFavorites); \
			if (PtrM->Flags & LFFlagNew) \
				Ops(LFContextNew); \
		}

#define ADD_STATS() \
	if (m_IsMainIndex) AddFileToStatistics(PtrM);

#define REMOVE_STATS() \
	if (m_IsMainIndex) RemoveFileFromStatistics(PtrM);

#define BUILD_ITEMDESCRIPTOR() \
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), m_AdditionalDataSize); \
	pItemDescriptor->Type = StoreFlagsToType(p_StoreDescriptor, LFTypeFile); \
	strcpy_s(pItemDescriptor->StoreID, LFKeySize, p_StoreDescriptor->StoreID);

#define APPEND_ITEMDESCRIPTOR() \
	LPVOID PtrS; \
	if (m_pTable[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS)) \
		AttachSlave(pItemDescriptor, PtrM->SlaveID, PtrS);

#define LOAD_MASTER(AbortOps, AbortRetval) \
	if (!LoadTable(IDXTABLE_MASTER)) { AbortOps; return AbortRetval; }

#define LOAD_SLAVE() \
	if ((PtrM->SlaveID) && (PtrM->SlaveID<IDXTABLECOUNT)) \
		if (LoadTable(PtrM->SlaveID)) {

#define DISCARD_SLAVE() } else { Result = m_pTable[PtrM->SlaveID]->GetError(); }

#define START_FINDMASTER(AbortOps, AbortRetval, Key) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT ID = 0; \
	LFCoreAttributes* PtrM; \
	if (m_pTable[IDXTABLE_MASTER]->FindKey(Key, ID, (LPVOID&)PtrM)) {

#define END_FINDMASTER() }

#define START_ITERATEMASTER(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT ID = 0; \
	LFCoreAttributes* PtrM; \
	while (m_pTable[IDXTABLE_MASTER]->FindNext(ID, (LPVOID&)PtrM)) {

#define END_ITERATEMASTER() }

#define START_ITERATEALL(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT IDs[IDXTABLECOUNT]; ZeroMemory(IDs, sizeof(IDs)); \
	LFCoreAttributes* PtrM; \
	while (m_pTable[IDXTABLE_MASTER]->FindNext(IDs[IDXTABLE_MASTER], (LPVOID&)PtrM)) {

#define END_ITERATEALL() }

#define IN_TRANSACTIONLIST(pTransactionList) \
	UINT ItemID = 0; \
	for (; ItemID<pTransactionList->m_ItemCount; ItemID++) \
	{ \
		if (((*pTransactionList)[ItemID].LastError==LFOk) && \
			(strcmp((*pTransactionList)[ItemID].StoreID, p_StoreDescriptor->StoreID)==0) && \
			(strcmp((*pTransactionList)[ItemID].FileID, PtrM->FileID)==0)) \
			goto Exists; \
	} \
	continue; \
	Exists:


// CIndex
//

CIndex::CIndex(CStore* pStore, BOOL IsMainIndex, BOOL WriteAccess, UINT StoreDataSize)
{
	assert(pStore);

	p_Store = pStore;
	p_StoreDescriptor = pStore->p_StoreDescriptor;
	m_WriteAccess = WriteAccess;
	m_AdditionalDataSize = StoreDataSize;

	ZeroMemory(m_pTable, sizeof(m_pTable));
	wcscpy_s(m_IdxPath, MAX_PATH, IsMainIndex ? p_StoreDescriptor->IdxPathMain : p_StoreDescriptor->IdxPathAux);
	m_IsMainIndex = IsMainIndex;
}

CIndex::~CIndex()
{
	for (UINT a=0; a<IDXTABLECOUNT; a++)
		if (m_pTable[a])
			delete m_pTable[a];
}

BOOL CIndex::LoadTable(UINT TableID, BOOL Initialize, UINT* pResult)
{
	assert(TableID<IDXTABLECOUNT);

	if (!m_pTable[TableID])
		m_pTable[TableID] = new CHeapfile(m_IdxPath, (BYTE)TableID, m_AdditionalDataSize, Initialize);

	const UINT Result = m_pTable[TableID]->m_OpenStatus;
	if (pResult)
		*pResult = Result;

	return (Result==HeapOk) || (Result==HeapCreated) || (Result==HeapMaintenanceRequired);
}

BOOL CIndex::Initialize()
{
	BOOL Result = TRUE;

	if (m_IdxPath[0]!=L'\0')
		for (UINT a=0; a<IDXTABLECOUNT; a++)
			if (!LoadTable(a, TRUE))
				Result = FALSE;

	return Result;
}

UINT CIndex::MaintenanceAndStatistics(BOOL Scheduled, BOOL* pRepaired, LFProgress* pProgress)
{
	assert(pRepaired);

	// Already performed non-scheduled maintenance?
	if (!Scheduled && (p_StoreDescriptor->Flags & LFStoreFlagsMaintained))
		return LFOk;

	// Is the index volume writeable?
	const BOOL Writeable = VolumeWriteable((CHAR)m_IdxPath[0]);
	if (Writeable)
	{
		p_StoreDescriptor->Flags |= LFStoreFlagsWriteable;
	}
	else
	{
		p_StoreDescriptor->Flags &= ~LFStoreFlagsWriteable;

		// Run scheduled maintenance only when index is writeable because of table compaction
		if (Scheduled)
			return LFDriveWriteProtected;
	}

	p_StoreDescriptor->Flags |= LFStoreFlagsMaintained;

	BOOL SlaveReindex = FALSE;
	BOOL UpdateContexts = FALSE;
	UINT TableLoadResult[IDXTABLECOUNT];
	UINT RecordSize = 0;

	// Check index tables
	for (UINT a=0; a<IDXTABLECOUNT; a++)
	{
		// Load table, but omit return value - we examine the open status directly here
		LoadTable(a, a>IDXTABLE_MASTER, &TableLoadResult[a]);

		switch (TableLoadResult[a])
		{
		case HeapCreated:
			if (a>IDXTABLE_MASTER)
			{
				*pRepaired = SlaveReindex = TRUE;
				break;
			}

			// Fall through

		case HeapError:
			p_StoreDescriptor->Flags |= LFStoreFlagsError;
			return LFIndexRepairError;

		case HeapMaintenanceRequired:
			COMPACT(a);

			*pRepaired = UpdateContexts = TRUE;
			break;

		case HeapNoAccess:
			p_StoreDescriptor->Flags |= LFStoreFlagsError;
			return LFIndexAccessError;

		case HeapSharingViolation:
			return LFSharingViolation1;

		case HeapCannotCreate:
			if (Writeable)
			{
				p_StoreDescriptor->Flags |= LFStoreFlagsError;
				return (a==IDXTABLE_MASTER) ? LFIndexRepairError : LFIndexCreateError;
			}

			return LFDriveWriteProtected;
		}

		// Max. record size for one file
		if (a!=IDXTABLE_MASTER)
		{
			const UINT ElementSize = m_pTable[a]->GetRequiredElementSize();

			if (ElementSize>RecordSize)
				RecordSize = ElementSize;
		}

		// Progress
		if (pProgress)
		{
			pProgress->MinorCurrent++;
			if (UpdateProgress(pProgress))
				return LFCancel;
		}
	}

	// Max. record size for one file
	RecordSize += m_pTable[IDXTABLE_MASTER]->GetRequiredElementSize();

	// Enough space for slave reindexing if neccessary?
	if (SlaveReindex)
		if (!RequiredSpaceAvailable(m_IdxPath, RecordSize*m_pTable[IDXTABLE_MASTER]->GetItemCount()))
			return LFNotEnoughFreeDiscSpace;

	// Reset statistics
	ResetStatistics();

	// Traverse index
	INT ID = 0;
	LFCoreAttributes* PtrM;
	while (m_pTable[IDXTABLE_MASTER]->FindNext(ID, (LPVOID&)PtrM))
	{
		// Operations below modify index
		if (!Writeable)
		{
			AddFileToStatistics(PtrM);
			continue;
		}

		WCHAR Path[2*MAX_PATH];
		if (Scheduled || SlaveReindex)
			p_Store->GetFileLocation(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), Path, 2*MAX_PATH);

		// File body present?
		if (Scheduled)
			if ((!(PtrM->Flags & LFFlagLink)) && LFIsStoreMounted(p_StoreDescriptor))
			{
				WIN32_FIND_DATA FindFileData;
				BOOL Exists = FileExists(Path, &FindFileData);

				// Update metadata
				if (Exists)
				{
					SetFromFindData(PtrM, &FindFileData);

					m_pTable[IDXTABLE_MASTER]->MakeDirty();
				}

				// Update flags
				UINT Flags = Exists ? 0 : LFFlagMissing;
				if ((Flags & LFFlagMissing)!=(PtrM->Flags & LFFlagMissing))
				{
					PtrM->Flags = (PtrM->Flags & ~LFFlagMissing) | Flags;
					*pRepaired = TRUE;

					m_pTable[IDXTABLE_MASTER]->MakeDirty();
				}
			}

		// Index version changed? Then update contexts!
		if (UpdateContexts)
		{
			SetFileContext(PtrM, TRUE);
			m_pTable[IDXTABLE_MASTER]->MakeDirty();
		}

		AddFileToStatistics(PtrM);

		// Slave index missing?
		if (SlaveReindex)
			if ((PtrM->SlaveID) && (PtrM->SlaveID<IDXTABLECOUNT))
				if (TableLoadResult[PtrM->SlaveID]==HeapCreated)
				{
					BUILD_ITEMDESCRIPTOR();

					SetAttributesFromFile(pItemDescriptor, &Path[4]);	// No fully qualified path allowed, skip prefix
					m_pTable[PtrM->SlaveID]->Add(pItemDescriptor);

					LFFreeItemDescriptor(pItemDescriptor);
				}
	}

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;
		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	// Compaction
	if (Scheduled)
		for (UINT a=0; a<IDXTABLECOUNT; a++)
		{
			COMPACT(a);

			*pRepaired |= (TableLoadResult[a]!=m_pTable[a]->m_OpenStatus);

			// Progress
			if (pProgress)
			{
				pProgress->MinorCurrent++;
				if (UpdateProgress(pProgress))
					return LFCancel;
			}
		}

	return LFOk;
}

void CIndex::AddFileToStatistics(LFCoreAttributes* PtrM) const
{
	assert(p_StoreDescriptor);
	assert(PtrM);

	#define ADD_FILE(Context) { p_StoreDescriptor->FileCount[Context]++; p_StoreDescriptor->FileSize[Context] += PtrM->FileSize; }
	COUNT_FILE(ADD_FILE);
}

void CIndex::RemoveFileFromStatistics(LFCoreAttributes* PtrM) const
{
	assert(p_StoreDescriptor);
	assert(PtrM);

	#define SUB_FILE(Context) { p_StoreDescriptor->FileCount[Context]--; p_StoreDescriptor->FileSize[Context] -= PtrM->FileSize; }
	COUNT_FILE(SUB_FILE);
}


// Operations
//

UINT CIndex::Add(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	// Master
	if (!LoadTable(IDXTABLE_MASTER))
		return LFIndexTableLoadError;

	// Slave
	if ((pItemDescriptor->CoreAttributes.SlaveID) && (pItemDescriptor->CoreAttributes.SlaveID<IDXTABLECOUNT))
	{
		if (!LoadTable(pItemDescriptor->CoreAttributes.SlaveID))
			return LFIndexTableLoadError;

		m_pTable[pItemDescriptor->CoreAttributes.SlaveID]->Add(pItemDescriptor);
	}

	m_pTable[IDXTABLE_MASTER]->Add(pItemDescriptor);

	if (m_IsMainIndex)
		AddFileToStatistics(&pItemDescriptor->CoreAttributes);

	return LFOk;
}

void CIndex::Query(LFFilter* pFilter, LFSearchResult* pSearchResult, BOOL UpdateStatistics)
{
	assert(pFilter);
	assert(pFilter->Mode>=LFFilterModeDirectoryTree);
	assert(pSearchResult);

	BOOL DoReset = TRUE;
	BYTE SearchtermContainsLetters = 0;

	START_ITERATEALL(pSearchResult->m_LastError = m_pTable[IDXTABLE_MASTER]->GetError(pFilter->StoreID[0]!=L'\0'),);

	if (UpdateStatistics)
	{
		// Delay resetting statistics in case of unaccessible index
		ResetStatistics(DoReset);

		AddFileToStatistics(PtrM);
	}

	BOOL CheckSearchterm = FALSE;
	if (!PassesFilter(IDXTABLE_MASTER, PtrM, pFilter, CheckSearchterm, SearchtermContainsLetters))
		continue;

	UINT Result = LFOk;
	LPVOID PtrS = NULL;

	if (!pFilter->Options.IgnoreSlaves)
	{
		LOAD_SLAVE();

		if (m_pTable[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
			if (!PassesFilter(PtrM->SlaveID, PtrS, pFilter, CheckSearchterm, SearchtermContainsLetters))
				continue;

		DISCARD_SLAVE();
	}

	if (Result==LFOk)
	{
		BUILD_ITEMDESCRIPTOR();
		if (PtrS)
			AttachSlave(pItemDescriptor, PtrM->SlaveID, PtrS);

		if (PassesFilter(pItemDescriptor, pFilter))
			if (pSearchResult->AddItem(pItemDescriptor))
				continue;

		LFFreeItemDescriptor(pItemDescriptor);
	}

	END_ITERATEALL();

	// Statistics reset still pending (happens if there weren't any files to iterate over in an empty store)?
	if (UpdateStatistics)
		ResetStatistics(DoReset);
}

UINT CIndex::UpdateStatistics()
{
	BOOL DoReset = TRUE;

	START_ITERATEALL(, m_pTable[IDXTABLE_MASTER]->GetError());

	// Delay resetting statistics in case of unaccessible index
	ResetStatistics(DoReset);

	AddFileToStatistics(PtrM);

	END_ITERATEALL();

	// Statistics reset still pending (happens if there weren't any files to iterate over in an empty store)?
	ResetStatistics(DoReset);

	return LFOk;
}

void CIndex::AddToSearchResult(LFTransactionList* pTransactionList, LFSearchResult* pSearchResult)
{
	assert(pTransactionList);
	assert(pSearchResult);

	START_ITERATEALL(pTransactionList->SetError(p_StoreDescriptor->StoreID, m_pTable[IDXTABLE_MASTER]->GetError()); pSearchResult->m_LastError = m_pTable[IDXTABLE_MASTER]->GetError(), );
	IN_TRANSACTIONLIST(pTransactionList);
	BUILD_ITEMDESCRIPTOR();

	UINT Result = LFOk;

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE();

	if (Result==LFOk)
		pSearchResult->AddItem(pItemDescriptor);

	pTransactionList->SetError(ItemID, Result);

	END_ITERATEALL();
}

void CIndex::ResolveLocations(LFTransactionList* pTransactionList)
{
	assert(pTransactionList);

	if (!LFIsStoreMounted(p_StoreDescriptor))
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFStoreNotMounted);
		return;
	}

	START_ITERATEMASTER(pTransactionList->SetError(p_StoreDescriptor->StoreID, m_pTable[IDXTABLE_MASTER]->GetError()),);
	IN_TRANSACTIONLIST(pTransactionList);

	pTransactionList->SetError(ItemID, p_Store->GetFileLocation(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), (*pTransactionList)[ItemID].Path, 2*MAX_PATH));

	END_ITERATEMASTER();
}

void CIndex::SendTo(LFTransactionList* pTransactionList, LPCSTR pStoreID, LFProgress* pProgress)
{
	assert(pTransactionList);

	if (!LFIsStoreMounted(p_StoreDescriptor))
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFStoreNotMounted);
		return;
	}

	UINT Result;

	// Store
	CHAR StoreID[LFKeySize] = "";
	if (pStoreID)
		strcpy_s(StoreID, LFKeySize, pStoreID);

	if (StoreID[0]=='\0')
		if ((Result=LFGetDefaultStore(StoreID))!=LFOk)
		{
			pTransactionList->SetError(p_StoreDescriptor->StoreID, Result);
			return;
		}

	if (strcmp(StoreID, p_StoreDescriptor->StoreID)==0)
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFOk);
		return;
	}

	CStore* pStore;
	if ((Result=OpenStore(StoreID, TRUE, &pStore))!=LFOk)
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, Result, pProgress);
		return;
	}

	START_ITERATEALL(pTransactionList->SetError(p_StoreDescriptor->StoreID, m_pTable[IDXTABLE_MASTER]->GetError()), );
	IN_TRANSACTIONLIST(pTransactionList);

	// Progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, PtrM->FileName);
		if (UpdateProgress(pProgress))
		{
			pTransactionList->m_LastError = LFCancel;
			return;
		}
	}

	BUILD_ITEMDESCRIPTOR();

	Result = LFOk;
	pItemDescriptor->CoreAttributes.Flags |= LFFlagNew;

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE();

	if (Result==LFOk)
	{
		WCHAR Path[2*MAX_PATH];
		if (p_Store->GetFileLocation(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), Path, 2*MAX_PATH)==LFOk)
			pTransactionList->SetError(ItemID, pStore->ImportFile(Path, pItemDescriptor, FALSE, FALSE), pProgress);
	}

	LFFreeItemDescriptor(pItemDescriptor);

	END_ITERATEALL();

	delete pStore;

	// Invalid items
	pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalID, pProgress);
}

BOOL CIndex::ExistingFileID(LPCSTR pFileID)
{
	assert(pFileID);

	BOOL Result = FALSE;

	START_FINDMASTER(, FALSE, pFileID);

	Result = TRUE;

	END_FINDMASTER();

	return Result;
}

BOOL CIndex::UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew)
{
	assert(pItemDescriptor);

	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	START_FINDMASTER(, Exists, pItemDescriptor->CoreAttributes.FileID);

	if (Exists || (pItemDescriptor->CoreAttributes.Flags & LFFlagLink))
	{
		pItemDescriptor->CoreAttributes.Flags &= ~LFFlagMissing;
	}
	else
	{
		pItemDescriptor->CoreAttributes.Flags |= LFFlagMissing;
	}

	if (RemoveNew)
		pItemDescriptor->CoreAttributes.Flags &= ~LFFlagNew;

	REMOVE_STATS();
	m_pTable[IDXTABLE_MASTER]->Update(pItemDescriptor, PtrM);
	ADD_STATS();

	END_FINDMASTER();

	return !(pItemDescriptor->CoreAttributes.Flags & LFFlagMissing);
}

void CIndex::UpdateItemState(LFTransactionList* pTransactionList, UINT Flags, FILETIME* pTransactionTime)
{
	assert(pTransactionList);
	assert(pTransactionTime);
	assert((Flags & ~(LFFlagArchive | LFFlagTrash))==0);

	// Access
	if (!m_WriteAccess)
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIndexAccessError);
		return;
	}

	START_ITERATEMASTER(pTransactionList->SetError(p_StoreDescriptor->StoreID, m_pTable[IDXTABLE_MASTER]->GetError()),);
	IN_TRANSACTIONLIST(pTransactionList);

	if ((PtrM->Flags & (LFFlagArchive | LFFlagTrash | LFFlagNew))!=Flags)
	{
		REMOVE_STATS();

		// Remove "New" flag
		PtrM->Flags &= ~LFFlagNew;

		// "Archive" flag
		if (Flags & LFFlagArchive)
			if (!(PtrM->Flags & LFFlagArchive))
			{
				PtrM->Flags |= LFFlagArchive;
				PtrM->ArchiveTime = *pTransactionTime;
			}

		// "Trash" flag
		if (Flags & LFFlagTrash)
			if (!(PtrM->Flags & LFFlagTrash))
			{
				PtrM->Flags |= LFFlagTrash;
				PtrM->DeleteTime = *pTransactionTime;
			}

		// Restore
		if (!Flags)
			if (PtrM->Flags & LFFlagTrash)
			{
				PtrM->Flags &= ~LFFlagTrash;
				ZeroMemory(&PtrM->DeleteTime, sizeof(FILETIME));
			}
			else
				if (PtrM->Flags & LFFlagArchive)
				{
					PtrM->Flags &= ~LFFlagArchive;
					ZeroMemory(&PtrM->ArchiveTime, sizeof(FILETIME));
				}

		m_pTable[IDXTABLE_MASTER]->MakeDirty();

		ADD_STATS();
	}

	pTransactionList->SetError(ItemID, LFOk);

	END_ITERATEMASTER();

	// Invalid items
	pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalID);
}

BOOL CIndex::InspectForUpdate(LFVariantData* pVariantData, BOOL& IncludeSlaves, BOOL& DoRename)
{
	if (pVariantData)
	{
		assert(pVariantData->Attr<LFAttributeCount);
		assert(pVariantData->Type==AttrProperties[pVariantData->Attr].Type);
		assert(pVariantData->Type<LFTypeCount);

		if (AttrProperties[pVariantData->Attr].ReadOnly)
			return FALSE;

		IncludeSlaves |= (pVariantData->Attr>LFLastCoreAttribute);
		DoRename |= (pVariantData->Attr==LFAttrFileName);
	}

	return TRUE;
}

void CIndex::Update(LFTransactionList* pTransactionList, LFVariantData* pVariantData1, LFVariantData* pVariantData2, LFVariantData* pVariantData3)
{
	assert(pTransactionList);

	// Access
	if (!m_WriteAccess)
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIndexAccessError);
		return;
	}

	// Inspect attributes
	BOOL IncludeSlaves = FALSE;
	BOOL DoRename = FALSE;

	if (!InspectForUpdate(pVariantData1, IncludeSlaves, DoRename) ||
		!InspectForUpdate(pVariantData2, IncludeSlaves, DoRename) ||
		!InspectForUpdate(pVariantData3, IncludeSlaves, DoRename))
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalAttribute);
		return;
	}

	// Start
	START_ITERATEALL(pTransactionList->SetError(p_StoreDescriptor->StoreID, m_pTable[IDXTABLE_MASTER]->GetError()),);
	IN_TRANSACTIONLIST(pTransactionList);
	REMOVE_STATS();

	LFItemDescriptor* pItemDescriptor = (*pTransactionList)[ItemID].pItemDescriptor;
	assert(pItemDescriptor);

	// Remove "New" flag
	pItemDescriptor->CoreAttributes.Flags &= ~LFFlagNew;

	// Update attributes
	if (pVariantData1)
		LFSetAttributeVariantData(pItemDescriptor, *pVariantData1);

	if (pVariantData2)
		LFSetAttributeVariantData(pItemDescriptor, *pVariantData2);

	if (pVariantData3)
		LFSetAttributeVariantData(pItemDescriptor, *pVariantData3);

	UINT Result = LFOk;

	// Phys. Datei umbenennen ?
	if (DoRename)
		if (!(PtrM->Flags & LFFlagLink) && m_IsMainIndex)
		{
			Result = p_Store->RenameFile(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), pItemDescriptor);

			switch (Result)
			{
			case LFOk:
				pItemDescriptor->CoreAttributes.Flags &= ~LFFlagMissing;
				break;

			case LFNoFileBody:
				pItemDescriptor->CoreAttributes.Flags |= LFFlagMissing;

			default:
				wcscpy_s(pItemDescriptor->CoreAttributes.FileName, 256, PtrM->FileName);

				if (m_AdditionalDataSize)
					memcpy_s(pItemDescriptor->StoreData, LFMaxStoreDataSize, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), m_AdditionalDataSize);
			}
		}

	// Master
	m_pTable[IDXTABLE_MASTER]->Update(pItemDescriptor, PtrM);

	ADD_STATS();

	// Slave
	if (IncludeSlaves)
	{
		LOAD_SLAVE();

		LPVOID PtrS;
		if (m_pTable[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
			m_pTable[PtrM->SlaveID]->Update(pItemDescriptor, PtrS);

		DISCARD_SLAVE();
	}

	pTransactionList->SetError(ItemID, Result);

	END_ITERATEALL();

	// Invalid items
	pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalID);
}

UINT CIndex::Synchronize(LFProgress* pProgress)
{
	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	UINT Result = LFOk;

	START_ITERATEALL(, m_pTable[IDXTABLE_MASTER]->GetError());

	// Progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, PtrM->FileName);
		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	if (p_Store->SynchronizeFile(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM)))
	{
		m_pTable[IDXTABLE_MASTER]->MakeDirty();

		// Progress
		if (pProgress)
		{
			pProgress->MinorCurrent++;
			if (UpdateProgress(pProgress))
				return LFCancel;
		}
	}
	else
	{
		LOAD_SLAVE();
		m_pTable[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);
		DISCARD_SLAVE();

		REMOVE_STATS();
		m_pTable[IDXTABLE_MASTER]->Invalidate(PtrM);

		// Progress
		if (pProgress)
			if (pProgress->UserAbort)
				return LFCancel;
	}

	END_ITERATEALL();

	return Result;
}

void CIndex::Delete(LFTransactionList* pTransactionList, LFProgress* pProgress)
{
	assert(pTransactionList);

	// Access
	if (!m_WriteAccess)
	{
		pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIndexAccessError);
		return;
	}

	START_ITERATEALL(pTransactionList->SetError(p_StoreDescriptor->StoreID, m_pTable[IDXTABLE_MASTER]->GetError()),);
	IN_TRANSACTIONLIST(pTransactionList);

	// Progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, PtrM->FileName);
		if (UpdateProgress(pProgress))
		{
			pTransactionList->m_LastError = LFCancel;
			return;
		}
	}

	UINT Result = (PtrM->Flags & LFFlagLink) ? LFOk : p_Store->DeleteFile(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM));

	if (Result==LFOk)
	{
		LOAD_SLAVE();
		m_pTable[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);
		DISCARD_SLAVE();
	}

	if (Result==LFOk)
	{
		REMOVE_STATS();
		m_pTable[IDXTABLE_MASTER]->Invalidate(PtrM);
	}

	pTransactionList->SetError(ItemID, Result, pProgress);

	// Progress
	if (pProgress)
		if (pProgress->UserAbort)
			return;

	END_ITERATEALL();

	// Invalid items
	pTransactionList->SetError(p_StoreDescriptor->StoreID, LFIllegalID, pProgress);
}
