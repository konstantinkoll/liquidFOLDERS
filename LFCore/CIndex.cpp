
#include "stdafx.h"
#include "CIndex.h"
#include "CStore.h"
#include "FileProperties.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "Query.h"
#include "Stores.h"
#include "TableAttributes.h"


// Macros
//

#define COMPACT(Index) \
	if (!RequiredSpaceAvailable(m_IdxPath, m_pTable[Index]->GetRequiredFileSize())) \
		return LFNotEnoughFreeDiscSpace; \
	if (!m_pTable[Index]->Compact()) \
		return LFIndexRepairError;

#define COUNT_FILE(ContextOps, TaskOps) \
	if (PtrM->Flags & LFFlagTrash) \
	{ \
		ContextOps(LFContextTrash); \
	} \
	else \
		if (PtrM->Flags & LFFlagArchive) \
		{ \
			ContextOps(LFContextArchive); \
		} \
		else \
		{ \
			if (!LFIsFilterFile(*PtrM)) \
				ContextOps(LFContextAllFiles); \
			const BYTE UserContext = LFGetUserContextID(*PtrM); \
			if (UserContext) \
				ContextOps(UserContext); \
			if (PtrM->Rating) \
				ContextOps(LFContextFavorites); \
			if (PtrM->Flags & LFFlagTask) \
			{ \
				ContextOps(LFContextTasks); \
				TaskOps(PtrM->Priority); \
			} \
			if (PtrM->Flags & LFFlagNew) \
				ContextOps(LFContextNew); \
		}

#define ADD_STATS() \
	if (m_IsMainIndex) AddFileToStatistics(PtrM);

#define REMOVE_STATS() \
	if (m_IsMainIndex) RemoveFileFromStatistics(PtrM);

#define BUILD_ITEMDESCRIPTOR() \
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor(PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), m_AdditionalDataSize); \
	pItemDescriptor->Type = m_StoreTypeFlags; \
	pItemDescriptor->StoreID = p_StoreDescriptor->StoreID;

#define APPEND_ITEMDESCRIPTOR() \
	LPVOID PtrS; \
	if (m_pTable[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS)) \
		AttachSlave(pItemDescriptor, PtrM->SlaveID, PtrS);

#define LOAD_MASTER(AbortOps, AbortRetval) \
	if (!LoadTable(IDXTABLE_MASTER)) { AbortOps; return AbortRetval; }

#define LOAD_SLAVE() \
	if (PtrM->SlaveID && (PtrM->SlaveID<IDXTABLECOUNT)) \
		if (LoadTable(PtrM->SlaveID)) {

#define DISCARD_SLAVE() } else { Result = m_pTable[PtrM->SlaveID]->GetError(); }

#define START_FINDMASTER(AbortOps, AbortRetval, Key) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT ID = 0; \
	LFCoreAttributes* PtrM; \
	if (m_pTable[IDXTABLE_MASTER]->FindKey(Key, ID, (LPVOID&)PtrM)) {

#define END_FINDMASTER() }

#define START_ITERATEMASTER() \
	LOAD_MASTER(SetError(pTransactionList, m_pTable[IDXTABLE_MASTER]->GetError()),); \
	INT ID = 0; \
	LFCoreAttributes* PtrM; \
	while (m_pTable[IDXTABLE_MASTER]->FindNext(ID, (LPVOID&)PtrM)) {

#define END_ITERATEMASTER() }

#define START_ITERATEALLEX(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT IDs[IDXTABLECOUNT]; ZeroMemory(IDs, sizeof(IDs)); \
	LFCoreAttributes* PtrM; \
	while (m_pTable[IDXTABLE_MASTER]->FindNext(IDs[IDXTABLE_MASTER], (LPVOID&)PtrM)) {

#define START_ITERATEALL(pTransactionList) \
	START_ITERATEALLEX(SetError(pTransactionList, m_pTable[IDXTABLE_MASTER]->GetError()),)

#define END_ITERATEALL() }

#define IN_TRANSACTIONLIST(pTransactionList) \
	UINT ItemID = 0; \
	for (; ItemID<pTransactionList->m_ItemCount; ItemID++) \
		if (((*pTransactionList)[ItemID].LastError==LFOk) && \
			((*pTransactionList)[ItemID].StoreID==p_StoreDescriptor->StoreID) && \
			((*pTransactionList)[ItemID].FileID==PtrM->FileID)) \
			goto Exists; \
	continue; \
	Exists:


// CIndex
//

CIndex::CIndex(CStore* pStore, BOOL IsMainIndex, BOOL WriteAccess, UINT StoreDataSize)
{
	assert(pStore);

	p_Store = pStore;
	m_StoreTypeFlags = StoreFlagsToType(p_StoreDescriptor=pStore->p_StoreDescriptor, LFTypeFile);
	m_WriteAccess = WriteAccess;
	m_AdditionalDataSize = StoreDataSize;

	ZeroMemory(m_pTable, sizeof(m_pTable));
	wcscpy_s(m_IdxPath, MAX_PATH, ((m_IsMainIndex=IsMainIndex)==TRUE) ? p_StoreDescriptor->IdxPathMain : p_StoreDescriptor->IdxPathAux);
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

UINT CIndex::MaintenanceAndStatistics(BOOL Scheduled, BOOL& Repaired, LFProgress* pProgress)
{
	// Already performed non-scheduled maintenance?
	if (!Scheduled && (p_StoreDescriptor->Flags & LFStoreFlagsMaintained))
		return LFOk;

	// Is the index volume writeable?
	const BOOL Writeable = VolumeWriteable((CHAR)m_IdxPath[0]);
	if (Writeable)
	{
		p_StoreDescriptor->Flags |= LFStoreFlagsManageable | LFStoreFlagsWriteable;
	}
	else
	{
		p_StoreDescriptor->Flags &= ~(LFStoreFlagsManageable | LFStoreFlagsWriteable);

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
				Repaired = SlaveReindex = TRUE;
				break;
			}

			// Fall through

		case HeapError:
			p_StoreDescriptor->Flags |= LFStoreFlagsError;

			return LFIndexRepairError;

		case HeapMaintenanceRequired:
			COMPACT(a);
			Repaired = UpdateContexts = TRUE;

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
			p_Store->GetFileLocation(*PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), Path, 2*MAX_PATH);

		// File body present?
		if (Scheduled)
			if (!(PtrM->Flags & LFFlagLink) && LFIsStoreMounted(p_StoreDescriptor))
			{
				WIN32_FIND_DATA FindFileData;
				const BOOL Exists = FileExists(Path, &FindFileData);

				// Update metadata
				if (Exists)
				{
					SetAttributesFromFindFileData(*PtrM, FindFileData);

					m_pTable[IDXTABLE_MASTER]->MakeDirty();
				}

				// Update flags
				const BYTE Flags = Exists ? 0 : LFFlagMissing;
				if ((Flags & LFFlagMissing)!=(PtrM->Flags & LFFlagMissing))
				{
					PtrM->Flags = (PtrM->Flags & ~LFFlagMissing) | Flags;
					Repaired = TRUE;

					m_pTable[IDXTABLE_MASTER]->MakeDirty();
				}
			}

		// Index version changed? Then update contexts!
		if (UpdateContexts)
		{
			SetFileContext(*PtrM, FALSE);
			m_pTable[IDXTABLE_MASTER]->MakeDirty();
		}

		AddFileToStatistics(PtrM);

		// Slave index missing?
		if (SlaveReindex)
			if (PtrM->SlaveID && (PtrM->SlaveID<IDXTABLECOUNT))
				if (TableLoadResult[PtrM->SlaveID]==HeapCreated)
				{
					BUILD_ITEMDESCRIPTOR();

					// Retrieve metadata
					SetAttributesFromShell(pItemDescriptor, &Path[4], FALSE);	// No fully qualified path allowed, skip prefix
					p_Store->SetAttributesFromStore(pItemDescriptor);

					// Add file to newly created slave
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

			Repaired |= (TableLoadResult[a]!=m_pTable[a]->m_OpenStatus);

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
	assert(PtrM);

	#define ADDFILE_CONTEXT(Context) { p_StoreDescriptor->Statistics.FileCount[Context]++; p_StoreDescriptor->Statistics.FileSize[Context] += PtrM->FileSize; }
	#define ADDFILE_TASK(Priority) { p_StoreDescriptor->Statistics.TaskCount[Priority]++; }
	COUNT_FILE(ADDFILE_CONTEXT, ADDFILE_TASK);
}

void CIndex::RemoveFileFromStatistics(LFCoreAttributes* PtrM) const
{
	assert(PtrM);

	#define SUBFILE_CONTEXT(Context) { p_StoreDescriptor->Statistics.FileCount[Context]--; p_StoreDescriptor->Statistics.FileSize[Context] -= PtrM->FileSize; }
	#define SUBFILE_TASK(Priority) { p_StoreDescriptor->Statistics.TaskCount[Priority]--; }
	COUNT_FILE(SUBFILE_CONTEXT, SUBFILE_TASK);
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
	if (pItemDescriptor->CoreAttributes.SlaveID && (pItemDescriptor->CoreAttributes.SlaveID<IDXTABLECOUNT))
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
	assert(pFilter->Query.Mode>=LFFilterModeDirectoryTree);
	assert(pSearchResult);

	BOOL DoReset = TRUE;

	BYTE QueryState;
	InitializeQueryState(QueryState);

	START_ITERATEALLEX(pSearchResult->m_LastError=m_pTable[IDXTABLE_MASTER]->GetError(pFilter->Query.StoreID[0]!=L'\0'),);

	if (UpdateStatistics)
	{
		// Delay resetting statistics in case of unaccessible index
		ResetStatistics(DoReset);

		AddFileToStatistics(PtrM);
	}

	ResetQueryState(QueryState);
	if (!PassesFilter(IDXTABLE_MASTER, PtrM, pFilter, QueryState))
		continue;

	UINT Result = LFOk;
	LPVOID PtrS = NULL;

	if (!pFilter->Query.IgnoreSlaves)
	{
		LOAD_SLAVE();

		if (m_pTable[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
			if (!PassesFilter(PtrM->SlaveID, PtrS, pFilter, QueryState))
				continue;

		DISCARD_SLAVE();
	}
	else
	{
		QueryState |= QUERYSTATE_PASSED_SLAVE;
	}

	if (Result==LFOk)
	{
		BUILD_ITEMDESCRIPTOR();
		if (PtrS)
			AttachSlave(pItemDescriptor, PtrM->SlaveID, PtrS);

		if (PassesFilter(pItemDescriptor, pFilter, QueryState) && pSearchResult->AddItem(pItemDescriptor))
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

	START_ITERATEALLEX(, m_pTable[IDXTABLE_MASTER]->GetError());

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

	START_ITERATEALLEX(SetError(pTransactionList, pSearchResult->m_LastError=m_pTable[IDXTABLE_MASTER]->GetError()), );
	IN_TRANSACTIONLIST(pTransactionList);
	BUILD_ITEMDESCRIPTOR();

	UINT Result = LFOk;

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE();

	if (Result==LFOk)
		LFAddItem(pSearchResult, pItemDescriptor);

	pTransactionList->SetError(ItemID, Result);

	END_ITERATEALL();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

void CIndex::ResolveLocations(LFTransactionList* pTransactionList)
{
	assert(pTransactionList);

	if (!LFIsStoreMounted(p_StoreDescriptor))
	{
		SetError(pTransactionList, LFStoreNotMounted);

		return;
	}

	START_ITERATEMASTER();
	IN_TRANSACTIONLIST(pTransactionList);

	pTransactionList->SetError(ItemID, p_Store->GetFileLocation(*PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), (*pTransactionList)[ItemID].Path, 2*MAX_PATH));

	END_ITERATEMASTER();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

void CIndex::SendTo(LFTransactionList* pTransactionList, const STOREID& StoreID, LFProgress* pProgress)
{
	assert(pTransactionList);

	if (!LFIsStoreMounted(p_StoreDescriptor))
	{
		SetError(pTransactionList, LFStoreNotMounted);

		return;
	}

	UINT Result;

	// Store
	ABSOLUTESTOREID AbsoluteStoreID;
	if ((Result=LFResolveStoreIDEx(AbsoluteStoreID, StoreID))!=LFOk)
	{
		SetError(pTransactionList, Result);

		return;
	}

	if (AbsoluteStoreID==p_StoreDescriptor->StoreID)
	{
		SetError(pTransactionList, LFOk);

		return;
	}

	CStore* pStore;
	if ((Result=OpenStore(AbsoluteStoreID, pStore))!=LFOk)
	{
		SetError(pTransactionList, Result, pProgress);

		return;
	}

	START_ITERATEALL(pTransactionList);
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

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE();

	//		if (!(PtrM->Flags & LFFlagLink) && m_IsMainIndex)

	if (Result==LFOk)
	{
		WCHAR Path[2*MAX_PATH];
		if (p_Store->GetFileLocation(*PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), Path, 2*MAX_PATH)==LFOk)
			pTransactionList->SetError(ItemID, pStore->ImportFile(Path, pItemDescriptor, FALSE, FALSE), pProgress);
	}

	LFFreeItemDescriptor(pItemDescriptor);

	END_ITERATEALL();

	delete pStore;

	// Invalid items
	SetError(pTransactionList, LFIllegalID, pProgress);
}

BOOL CIndex::ExistingFileID(const FILEID& FileID)
{
	BOOL Result = FALSE;

	START_FINDMASTER(, FALSE, FileID);

	Result = TRUE;

	END_FINDMASTER();

	return Result;
}

void CIndex::UpdateUserContext(LFTransactionList* pTransactionList, BYTE UserContextID)
{
	assert(pTransactionList);

	// Access
	if (!m_WriteAccess)
	{
		SetError(pTransactionList, LFIndexAccessError);

		return;
	}

	START_ITERATEMASTER();
	IN_TRANSACTIONLIST(pTransactionList);

	UINT Result;

	// Move allowed?
	if (!ContextMoveAllowed(PtrM->SystemContextID, UserContextID))
	{
		Result = LFIllegalItemType;
	}
	else
	{
		Result = LFOk;

		REMOVE_STATS();

		// Remove "New" flag
		PtrM->Flags &= ~LFFlagNew;

		// Set new user context (reset if same as system context)
		PtrM->UserContextID = (PtrM->SystemContextID==UserContextID) ? 0 : UserContextID;

		m_pTable[IDXTABLE_MASTER]->MakeDirty();
	
		ADD_STATS();
	}

	pTransactionList->SetError(ItemID, Result);

	END_ITERATEMASTER();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

BOOL CIndex::UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew)
{
	assert(pItemDescriptor);

	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	START_FINDMASTER(, Exists, pItemDescriptor->CoreAttributes.FileID);

	if (Exists || (PtrM->Flags & LFFlagLink))
	{
		PtrM->Flags &= ~LFFlagMissing;
	}
	else
	{
		PtrM->Flags |= LFFlagMissing;
	}

	if (RemoveNew)
		PtrM->Flags &= ~LFFlagNew;

	REMOVE_STATS();
	pItemDescriptor->CoreAttributes.Flags = PtrM->Flags;
	m_pTable[IDXTABLE_MASTER]->MakeDirty();
	ADD_STATS();

	END_FINDMASTER();

	return !(pItemDescriptor->CoreAttributes.Flags & LFFlagMissing);
}

void CIndex::UpdateItemState(LFTransactionList* pTransactionList, const FILETIME& TransactionTime, BYTE Flags)
{
	assert(pTransactionList);
	assert((Flags & ~(LFFlagTrash | LFFlagArchive))==0);

	// Access
	if (!m_WriteAccess)
	{
		SetError(pTransactionList, LFIndexAccessError);

		return;
	}

	START_ITERATEMASTER();
	IN_TRANSACTIONLIST(pTransactionList);

	if ((PtrM->Flags & (LFFlagTrash | LFFlagTask | LFFlagArchive | LFFlagNew))!=Flags)
	{
		REMOVE_STATS();

		// Remove "New" flag
		PtrM->Flags &= ~LFFlagNew;

		// "Archive" flag
		if (Flags & LFFlagArchive)
			if (!(PtrM->Flags & LFFlagArchive))
			{
				PtrM->Flags |= LFFlagArchive;
				PtrM->ArchiveTime = TransactionTime;
			}

		// "Trash" flag
		if (Flags & LFFlagTrash)
			if (!(PtrM->Flags & LFFlagTrash))
			{
				PtrM->Flags |= LFFlagTrash;
				PtrM->DeleteTime = TransactionTime;
			}

		// Recover
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
				else
					if (PtrM->Flags & LFFlagTask)
					{
						PtrM->Flags &= ~LFFlagTask;

						// Check if data structure is large enough for properties that were extended in liquidFOLDERS 3.1.0
						if (m_pTable[IDXTABLE_MASTER]->GetVersion()>=4)
						{
							assert(m_pTable[IDXTABLE_MASTER]->GetElementSize()>=offsetof(LFCoreAttributes, DueTime)+sizeof(FILETIME));
							assert(m_pTable[IDXTABLE_MASTER]->GetElementSize()>=offsetof(LFCoreAttributes, DoneTime)+sizeof(FILETIME));

							PtrM->DueTime.dwHighDateTime = PtrM->DueTime.dwLowDateTime = 0;
							PtrM->DoneTime = TransactionTime;
						}
					}

		m_pTable[IDXTABLE_MASTER]->MakeDirty();

		ADD_STATS();
	}

	pTransactionList->SetError(ItemID, LFOk);

	END_ITERATEMASTER();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

BOOL CIndex::InspectForUpdate(const LFVariantData* pVData, BOOL& IncludeSlaves, BOOL& DoRename)
{
	if (pVData)
	{
		assert(pVData->Attr<LFAttributeCount);
		assert(pVData->Type==AttrProperties[pVData->Attr].Type);
		assert(pVData->Type<LFTypeCount);

		if (!(AttrProperties[pVData->Attr].DataFlags & LFDataEditable))
			return FALSE;

		IncludeSlaves |= (pVData->Attr>LFLastCoreAttribute);
		DoRename |= (pVData->Attr==LFAttrFileName);
	}

	return TRUE;
}

void CIndex::Update(LFTransactionList* pTransactionList, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2, const LFVariantData* pVariantData3, BOOL MakeTask)
{
	assert(pTransactionList);

	// Access
	if (!m_WriteAccess)
	{
		SetError(pTransactionList, LFIndexAccessError);

		return;
	}

	// Inspect attributes
	BOOL IncludeSlaves = FALSE;
	BOOL DoRename = FALSE;

	if (!InspectForUpdate(pVariantData1, IncludeSlaves, DoRename) ||
		!InspectForUpdate(pVariantData2, IncludeSlaves, DoRename) ||
		!InspectForUpdate(pVariantData3, IncludeSlaves, DoRename))
	{
		SetError(pTransactionList, LFIllegalAttribute);

		return;
	}

	// Start
	START_ITERATEALL(pTransactionList);
	IN_TRANSACTIONLIST(pTransactionList);

	UINT Result;

	// Write protected?
	if (PtrM->Flags & (LFFlagArchive | LFFlagTrash))
	{
		Result = LFFileWriteProtected;
	}
	else
	{
		Result = LFOk;

		REMOVE_STATS();

		LFItemDescriptor* pItemDescriptor = (*pTransactionList)[ItemID].pItemDescriptor;
		assert(pItemDescriptor);

		// Remove "New" flag
		pItemDescriptor->CoreAttributes.Flags &= ~LFFlagNew;

		// "Task" flag
		if (MakeTask)
		{
			pItemDescriptor->CoreAttributes.Flags |= LFFlagTask;
			pItemDescriptor->CoreAttributes.DoneTime.dwHighDateTime = pItemDescriptor->CoreAttributes.DoneTime.dwLowDateTime = 0;
		}

		// Update attributes
		if (pVariantData1)
			LFSetAttributeVariantData(pItemDescriptor, *pVariantData1);

		if (pVariantData2)
			LFSetAttributeVariantData(pItemDescriptor, *pVariantData2);

		if (pVariantData3)
			LFSetAttributeVariantData(pItemDescriptor, *pVariantData3);

		// Rename physical file?
		if (DoRename)
			if (!(PtrM->Flags & LFFlagLink) && m_IsMainIndex)
			{
				Result = p_Store->RenameFile(*PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), pItemDescriptor);

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
#ifdef _DEBUG
						memcpy_s(pItemDescriptor->StoreData, LFMaxStoreDataSize, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), m_AdditionalDataSize);
#else
						memcpy(pItemDescriptor->StoreData, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM), m_AdditionalDataSize);
#endif
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
	}

	pTransactionList->SetError(ItemID, Result);

	END_ITERATEALL();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

UINT CIndex::Synchronize(LFProgress* pProgress)
{
	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	UINT Result = LFOk;

	START_ITERATEALLEX(, m_pTable[IDXTABLE_MASTER]->GetError());

	// Progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, PtrM->FileName);
		if (UpdateProgress(pProgress))
			return LFCancel;
	}

	if (p_Store->SynchronizeFile(*PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM)))
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
		SetError(pTransactionList, LFIndexAccessError);

		return;
	}

	START_ITERATEALL(pTransactionList);
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

	UINT Result = (PtrM->Flags & LFFlagLink) ? LFOk : p_Store->DeleteFile(*PtrM, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrM));

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
	SetError(pTransactionList, LFIllegalID, pProgress);
}
