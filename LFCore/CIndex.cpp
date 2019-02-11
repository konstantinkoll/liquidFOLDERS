
#include "stdafx.h"
#include "CIndex.h"
#include "CStore.h"
#include "FileProperties.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "Query.h"
#include "Stores.h"
#include "TableAttributes.h"


extern LFMessageIDs LFMessages;


// Macros
//

#define ADD_STATS(ITEM) \
	if (m_IsMainIndex) p_Store->AddFileToStatistics(ITEM);

#define REMOVE_STATS(ITEM) \
	if (m_IsMainIndex) p_Store->RemoveFileFromStatistics(ITEM);

#define BUILD_ITEMDESCRIPTOR() \
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor(PtrMaster, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster), m_StoreDataSize); \
	pItemDescriptor->Type = m_StoreTypeFlags; \
	pItemDescriptor->StoreID = p_StoreDescriptor->StoreID;

#define APPEND_ITEMDESCRIPTOR() \
	LPVOID PtrSlave = NULL; \
	if (m_pTable[PtrMaster->SlaveID]->FindKey(PtrMaster->FileID, IDs[PtrMaster->SlaveID], PtrSlave)) \
		AttachSlave(pItemDescriptor, PtrMaster->SlaveID, PtrSlave);

#define LOAD_MASTER(AbortOps, AbortRetval) \
	if (!LoadTable(IDXTABLE_MASTER)) { AbortOps; return AbortRetval; }

#define LOAD_SLAVE() \
	if (PtrMaster->SlaveID && (PtrMaster->SlaveID<IDXTABLECOUNT)) \
		if (LoadTable(PtrMaster->SlaveID)) {

#define DISCARD_SLAVE() } else { Result = m_pTable[PtrMaster->SlaveID]->GetError(); }

#define START_FINDMASTER(AbortOps, AbortRetval, Key) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT_PTR ID = 0; \
	LPCOREATTRIBUTES PtrMaster = NULL; \
	if (m_pTable[IDXTABLE_MASTER]->FindKey(Key, ID, (LPVOID&)PtrMaster)) {

#define END_FINDMASTER() }

#define START_ITERATEMASTEREX(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT_PTR ID = 0; \
	LPCOREATTRIBUTES PtrMaster = NULL; \
	while (m_pTable[IDXTABLE_MASTER]->FindNext(ID, (LPVOID&)PtrMaster)) {

#define START_ITERATEMASTER() \
	START_ITERATEMASTEREX(SetError(pTransactionList, m_pTable[IDXTABLE_MASTER]->GetError()),)

#define END_ITERATEMASTER() }

#define START_ITERATEALLEX(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT_PTR IDs[IDXTABLECOUNT]; ZeroMemory(IDs, sizeof(IDs)); \
	LPCOREATTRIBUTES PtrMaster = NULL; \
	while (m_pTable[IDXTABLE_MASTER]->FindNext(IDs[IDXTABLE_MASTER], (LPVOID&)PtrMaster)) {

#define START_ITERATEALL(pTransactionList) \
	START_ITERATEALLEX(SetError(pTransactionList, m_pTable[IDXTABLE_MASTER]->GetError()),)

#define END_ITERATEALL() }

#define IN_TRANSACTIONLIST(pTransactionList) \
	UINT ItemID = 0; \
	for (; ItemID<pTransactionList->m_ItemCount; ItemID++) \
		if (((*pTransactionList)[ItemID].LastError==LFOk) && \
			((*pTransactionList)[ItemID].StoreID==p_StoreDescriptor->StoreID) && \
			((*pTransactionList)[ItemID].FileID==PtrMaster->FileID)) \
			goto Exists; \
	continue; \
	Exists:


// CIndex
//

CIndex::CIndex(CStore* pStore, BOOL IsMainIndex, BOOL WriteAccess, UINT StoreDataSize)
{
	assert(pStore);
	assert(StoreDataSize<=LFMaxStoreDataSize);

	p_Store = pStore;
	m_StoreTypeFlags = StoreFlagsToType(p_StoreDescriptor=pStore->p_StoreDescriptor, LFTypeFile);
	m_StoreDataSize = StoreDataSize;

	ZeroMemory(m_pTable, sizeof(m_pTable));
	wcscpy_s(m_IdxPath, MAX_PATH, ((m_IsMainIndex=IsMainIndex)==TRUE) ? p_StoreDescriptor->IdxPathMain : p_StoreDescriptor->IdxPathAux);

	m_WriteAccess = ((m_VolumeWriteable=VolumeWriteable((CHAR)m_IdxPath[0]))==TRUE) && WriteAccess;
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
		m_pTable[TableID] = new CHeapfile(m_IdxPath, (BYTE)TableID, m_StoreDataSize, Initialize);

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

UINT CIndex::CompactTable(UINT TableID) const
{
	assert(TableID<IDXTABLECOUNT);

	if (!RequiredSpaceAvailable(m_IdxPath, m_pTable[TableID]->GetRequiredFileSize()))
		return LFNotEnoughFreeDiscSpace;

	return m_pTable[TableID]->Compact() ? LFOk : LFIndexRepairError;
}

UINT CIndex::MaintenanceAndStatistics(BOOL Scheduled, BOOL& Repaired, LFProgress* pProgress)
{
	// Already performed non-scheduled maintenance?
	if (!Scheduled && (p_StoreDescriptor->Flags & LFStoreFlagsMaintained))
		return LFOk;

	// Update store flags
	if (m_VolumeWriteable)
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
			UINT Result;
			if ((Result=CompactTable(a))!=LFOk)
				return Result;

			Repaired = UpdateContexts = TRUE;

			break;

		case HeapNoAccess:
			p_StoreDescriptor->Flags |= LFStoreFlagsError;

			return LFIndexAccessError;

		case HeapSharingViolation:
			return LFSharingViolation1;

		case HeapCannotCreate:
			if (m_WriteAccess)
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
		if (ProgressMinorNext(pProgress))
			return LFCancel;
	}

	// Max. record size for one file
	RecordSize += m_pTable[IDXTABLE_MASTER]->GetRequiredElementSize();

	// Enough space for slave reindexing if neccessary?
	if (SlaveReindex && !RequiredSpaceAvailable(m_IdxPath, RecordSize*m_pTable[IDXTABLE_MASTER]->GetItemCount()))
		return LFNotEnoughFreeDiscSpace;

	// Reset statistics
	p_Store->ResetStatistics();

	// Traverse index
	INT_PTR ID = 0;
	LPCOREATTRIBUTES PtrMaster = NULL;
	while (m_pTable[IDXTABLE_MASTER]->FindNext(ID, (LPVOID&)PtrMaster))
	{
		// Operations below modify index, so the index has to be writeable
		if (m_WriteAccess)
		{
			// Index version changed? Then update contexts!
			if (UpdateContexts)
			{
				SetFileContext(*PtrMaster, FALSE);
				m_pTable[IDXTABLE_MASTER]->MakeDirty();
			}

			// Get file location for reindexing of slave, and for verifying location on scheduled maintenance
			if (Scheduled || SlaveReindex)
			{
				const BYTE Flags = PtrMaster->Flags;

				WCHAR Path[2*MAX_PATH];
				WIN32_FIND_DATA FindData;
				if (GetFileLocation(PtrMaster, Path, 2*MAX_PATH, NULL, FALSE, &FindData)==LFOk)
				{
					// Flags changed?
					Repaired |= (Flags!=PtrMaster->Flags);

					if (Scheduled)
					{
						// Update metadata
						SetAttributesFromFindData(*PtrMaster, FindData);
						m_pTable[IDXTABLE_MASTER]->MakeDirty();
					}

					// Slave index missing?
					if (SlaveReindex && PtrMaster->SlaveID && (PtrMaster->SlaveID<IDXTABLECOUNT) && (TableLoadResult[PtrMaster->SlaveID]==HeapCreated))
					{
						BUILD_ITEMDESCRIPTOR();

						// Retrieve metadata
						SetAttributesFromShell(pItemDescriptor, &Path[4], FALSE);	// No fully qualified path allowed, skip prefix
						p_Store->SetAttributesFromStore(pItemDescriptor);

						// Add file to newly created slave
						m_pTable[PtrMaster->SlaveID]->Add(pItemDescriptor);
	
						LFFreeItemDescriptor(pItemDescriptor);
					}
				}
			}
		}

		// Add file to statistics after all maintenance checks are completed
		p_Store->AddFileToStatistics(PtrMaster);
	}

	// Progress
	if (ProgressMinorNext(pProgress))
		return LFCancel;

	// Compaction
	if (Scheduled)
		for (UINT a=0; a<IDXTABLECOUNT; a++)
		{
			UINT Result;
			if ((Result=CompactTable(a))!=LFOk)
				return Result;

			Repaired |= (TableLoadResult[a]!=m_pTable[a]->m_OpenStatus);

			// Progress
			if (ProgressMinorNext(pProgress))
				return LFCancel;
		}

	return LFOk;
}

UINT CIndex::GetFileLocation(const HORCRUXFILE& File, LPWSTR pPath, SIZE_T cCount, LFItemDescriptor* pItemDescriptor, BOOL RemoveNew, WIN32_FIND_DATA* pFindData)
{
	assert(pPath);
	assert(cCount>=2*MAX_PATH);

	UINT Result;
	if ((Result=p_Store->GetFilePath(File, pPath, cCount))!=LFOk)
		return Result;

	const BOOL Exists = FileExists(pPath, pFindData);

	// Update flags
	RemoveNew &= m_WriteAccess && ((File & LFFlagNew)!=0);

	if (m_WriteAccess)
		if ((Exists!=((File & LFFlagMissing)==0)) || RemoveNew)
		{
			if (Exists)
			{
				File -= LFFlagMissing;
			}
			else
			{
				File += LFFlagMissing;
			}

			if (RemoveNew)
				File -= LFFlagNew;

			m_pTable[IDXTABLE_MASTER]->MakeDirty();
		}

	// Update item descriptor
	if (pItemDescriptor)
	{
		pItemDescriptor->CoreAttributes.Flags = File;

		if (m_StoreDataSize)
			memcpy(pItemDescriptor->StoreData, File, m_StoreDataSize);

		if (RemoveNew && m_IsMainIndex)
			SendLFNotifyMessage(LFMessages.StatisticsChanged);
	}

	return Exists ? LFOk : LFNoFileBody;
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

	ADD_STATS(pItemDescriptor);

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
		p_Store->ResetStatistics(DoReset);
		p_Store->AddFileToStatistics(PtrMaster);
	}

	ResetQueryState(QueryState);
	if (!PassesFilter(IDXTABLE_MASTER, PtrMaster, pFilter, QueryState))
		continue;

	UINT Result = LFOk;
	LPVOID PtrSlave = NULL;

	if (!pFilter->Query.IgnoreSlaves)
	{
		LOAD_SLAVE();

		if (m_pTable[PtrMaster->SlaveID]->FindKey(PtrMaster->FileID, IDs[PtrMaster->SlaveID], PtrSlave))
			if (!PassesFilter(PtrMaster->SlaveID, PtrSlave, pFilter, QueryState))
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
		if (PtrSlave)
			AttachSlave(pItemDescriptor, PtrMaster->SlaveID, PtrSlave);

		if (PassesFilter(pItemDescriptor, pFilter, QueryState) && pSearchResult->AddItem(pItemDescriptor))
			continue;

		LFFreeItemDescriptor(pItemDescriptor);
	}

	END_ITERATEALL();

	// Statistics reset still pending (happens if there weren't any files to iterate over in an empty store)?
	if (UpdateStatistics)
		p_Store->ResetStatistics(DoReset);
}

UINT CIndex::UpdateStatistics()
{
	BOOL DoReset = TRUE;

	START_ITERATEALLEX(, m_pTable[IDXTABLE_MASTER]->GetError());

	// Delay resetting statistics in case of unaccessible index
	p_Store->ResetStatistics(DoReset);
	p_Store->AddFileToStatistics(PtrMaster);

	END_ITERATEALL();

	// Statistics reset still pending (happens if there weren't any files to iterate over in an empty store)?
	p_Store->ResetStatistics(DoReset);

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

	SetError(pTransactionList, ItemID, Result);

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

	SetError(pTransactionList, ItemID, GetFileLocation(PtrMaster, (*pTransactionList)[ItemID].Path, 2*MAX_PATH, (*pTransactionList)[ItemID].pItemDescriptor));

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
	if (SetProgressObject(pProgress, PtrMaster->FileName))
	{
		pTransactionList->m_LastError = LFCancel;

		return;
	}

	BUILD_ITEMDESCRIPTOR();

	Result = LFOk;

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE();

	if (Result==LFOk)
	{
		WCHAR Path[2*MAX_PATH];
		if (GetFileLocation(PtrMaster, Path, 2*MAX_PATH, (*pTransactionList)[ItemID].pItemDescriptor)==LFOk)
			SetError(pTransactionList, ItemID, pStore->ImportFile(Path, pItemDescriptor, FALSE, FALSE), pProgress);
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
	if (!ContextMoveAllowed(PtrMaster->SystemContextID, UserContextID))
	{
		Result = LFIllegalItemType;
	}
	else
	{
		Result = LFOk;

		REMOVE_STATS(PtrMaster);

		// Remove "New" flag
		PtrMaster->Flags &= ~LFFlagNew;

		// Set new user context (reset if same as system context)
		PtrMaster->UserContextID = (PtrMaster->SystemContextID==UserContextID) ? 0 : UserContextID;

		m_pTable[IDXTABLE_MASTER]->MakeDirty();
	
		ADD_STATS(PtrMaster);
	}

	SetError(pTransactionList, ItemID, Result);

	END_ITERATEMASTER();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

UINT CIndex::GetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount, BOOL RemoveNew)
{
	assert(pItemDescriptor);

	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	UINT Result = LFIllegalID;

	START_FINDMASTER(, LFIllegalID, pItemDescriptor->CoreAttributes.FileID);

	REMOVE_STATS(PtrMaster);
	Result = GetFileLocation(PtrMaster, pPath, cCount, pItemDescriptor, RemoveNew);
	m_pTable[IDXTABLE_MASTER]->MakeDirty();
	ADD_STATS(PtrMaster);

	END_FINDMASTER();

	return Result;
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

	if ((PtrMaster->Flags & (LFFlagTrash | LFFlagTask | LFFlagArchive | LFFlagNew))!=Flags)
	{
		REMOVE_STATS(PtrMaster);

		// Remove "New" flag
		PtrMaster->Flags &= ~LFFlagNew;

		// "Archive" flag
		if (Flags & LFFlagArchive)
			if (!(PtrMaster->Flags & LFFlagArchive))
			{
				PtrMaster->Flags |= LFFlagArchive;
				PtrMaster->ArchiveTime = TransactionTime;
			}

		// "Trash" flag
		if (Flags & LFFlagTrash)
			if (!(PtrMaster->Flags & LFFlagTrash))
			{
				PtrMaster->Flags |= LFFlagTrash;
				PtrMaster->DeleteTime = TransactionTime;
			}

		// Recover
		if (!Flags)
			if (PtrMaster->Flags & LFFlagTrash)
			{
				PtrMaster->Flags &= ~LFFlagTrash;
				ZeroMemory(&PtrMaster->DeleteTime, sizeof(FILETIME));
			}
			else
				if (PtrMaster->Flags & LFFlagArchive)
				{
					PtrMaster->Flags &= ~LFFlagArchive;
					ZeroMemory(&PtrMaster->ArchiveTime, sizeof(FILETIME));
				}
				else
					if (PtrMaster->Flags & LFFlagTask)
					{
						PtrMaster->Flags &= ~LFFlagTask;

						// Check if data structure is large enough for properties that were extended in liquidFOLDERS 3.1.0
						if (m_pTable[IDXTABLE_MASTER]->GetVersion()>=4)
						{
							assert(m_pTable[IDXTABLE_MASTER]->GetElementSize()>=offsetof(LFCoreAttributes, DueTime)+sizeof(FILETIME));
							assert(m_pTable[IDXTABLE_MASTER]->GetElementSize()>=offsetof(LFCoreAttributes, DoneTime)+sizeof(FILETIME));

							PtrMaster->DueTime.dwHighDateTime = PtrMaster->DueTime.dwLowDateTime = 0;
							PtrMaster->DoneTime = TransactionTime;
						}
					}

		m_pTable[IDXTABLE_MASTER]->MakeDirty();

		ADD_STATS(PtrMaster);
	}

	SetError(pTransactionList, ItemID, LFOk, NULL);

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
	if (PtrMaster->Flags & (LFFlagArchive | LFFlagTrash))
	{
		Result = LFFileWriteProtected;
	}
	else
	{
		Result = LFOk;

		REMOVE_STATS(PtrMaster);

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
		if (DoRename && m_IsMainIndex)
			switch (Result=p_Store->RenameFile(HORCRUXFILE(PtrMaster, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster)), pItemDescriptor))
			{
			case LFOk:
				pItemDescriptor->CoreAttributes.Flags &= ~LFFlagMissing;
				break;

			case LFNoFileBody:
				pItemDescriptor->CoreAttributes.Flags |= LFFlagMissing;

			default:
				// Revert renaming operation
				wcscpy_s(pItemDescriptor->CoreAttributes.FileName, 256, PtrMaster->FileName);

				if (m_StoreDataSize)
					memcpy(pItemDescriptor->StoreData, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster), m_StoreDataSize);
			}

		// Master
		m_pTable[IDXTABLE_MASTER]->Update(pItemDescriptor, PtrMaster);

		ADD_STATS(PtrMaster);

		// Slave
		if (IncludeSlaves)
		{
			LOAD_SLAVE();

			LPVOID PtrSlave = NULL;
			if (m_pTable[PtrMaster->SlaveID]->FindKey(PtrMaster->FileID, IDs[PtrMaster->SlaveID], PtrSlave))
				m_pTable[PtrMaster->SlaveID]->Update(pItemDescriptor, PtrSlave);

			DISCARD_SLAVE();
		}
	}

	SetError(pTransactionList, ItemID, Result);

	END_ITERATEALL();

	// Invalid items
	SetError(pTransactionList, LFIllegalID);
}

UINT CIndex::SynchronizeMatch()
{
	UINT Result = LFOk;

	START_ITERATEMASTEREX(, m_pTable[IDXTABLE_MASTER]->GetError());

	p_Store->SynchronizeMatch(HORCRUXFILE(PtrMaster, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster)));

	END_ITERATEMASTER();

	return Result;
}

UINT CIndex::SynchronizeCommit(LFProgress* pProgress)
{
	// Access
	if (!m_WriteAccess)
		return LFIndexAccessError;

	UINT Result = LFOk;

	START_ITERATEALLEX(, m_pTable[IDXTABLE_MASTER]->GetError());

	// Progress
	if (SetProgressObject(pProgress, PtrMaster->FileName))
		return LFCancel;

	if (p_Store->SynchronizeCommit(HORCRUXFILE(PtrMaster, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster))))
	{
		// File is not missing
		PtrMaster->Flags &= ~LFFlagMissing;

		// Update core attributes from find data
		m_pTable[IDXTABLE_MASTER]->MakeDirty();

		// Progress
		if (ProgressMinorNext(pProgress))
			return LFCancel;
	}
	else
	{
		// Remove file from index
		LOAD_SLAVE();
		m_pTable[PtrMaster->SlaveID]->Invalidate(PtrMaster->FileID, IDs[PtrMaster->SlaveID]);
		DISCARD_SLAVE();

		REMOVE_STATS(PtrMaster);
		m_pTable[IDXTABLE_MASTER]->Invalidate(PtrMaster);

		// Progress
		if (AbortProgress(pProgress))
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
	if (SetProgressObject(pProgress, PtrMaster->FileName))
	{
		pTransactionList->m_LastError = LFCancel;

		return;
	}

	UINT Result = p_Store->DeleteFile(HORCRUXFILE(PtrMaster, m_pTable[IDXTABLE_MASTER]->GetStoreData(PtrMaster)));
	if (Result==LFOk)
	{
		LOAD_SLAVE();
		m_pTable[PtrMaster->SlaveID]->Invalidate(PtrMaster->FileID, IDs[PtrMaster->SlaveID]);
		DISCARD_SLAVE();
	}

	if (Result==LFOk)
	{
		REMOVE_STATS(PtrMaster);
		m_pTable[IDXTABLE_MASTER]->Invalidate(PtrMaster);
	}

	SetError(pTransactionList, ItemID, Result, pProgress);

	// Progress
	if (AbortProgress(pProgress))
		return;

	END_ITERATEALL();

	// Invalid items
	SetError(pTransactionList, LFIllegalID, pProgress);
}
