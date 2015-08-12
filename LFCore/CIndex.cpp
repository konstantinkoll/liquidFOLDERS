
#include "stdafx.h"
#include "CIndex.h"
#include "LFItemDescriptor.h"
#include "Query.h"
#include "ShellProperties.h"
#include "StoreCache.h"
#include "Stores.h"
#include <assert.h>


// Macros
//

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
			Ops(LFContextAllFiles); \
			if (PtrM->Rating) \
				Ops(LFContextFavorites); \
			if (PtrM->Flags & LFFlagNew) \
				Ops(LFContextNew); \
			Ops(PtrM->ContextID); \
		}

#define ADD_STATS() \
	if (TrackStats) AddFileToStatistics(PtrM);

#define REMOVE_STATS() \
	if (TrackStats) RemoveFileFromStatistics(PtrM);

#define BUILD_ITEMDESCRIPTOR() \
	i = LFAllocItemDescriptor(PtrM); \
	i->Type = Type; \
	strcpy_s(i->StoreID, LFKeySize, slot->StoreID);

#define APPEND_ITEMDESCRIPTOR() \
	void* PtrS; \
	if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS)) \
		AttachSlave(i, PtrM->SlaveID, PtrS);

#define LOAD_MASTER(AbortOps, AbortRetval) \
	if (!LoadTable(IDXTABLE_MASTER)) { AbortOps; return AbortRetval; }

#define LOAD_SLAVE() \
	if ((PtrM->SlaveID) && (PtrM->SlaveID<IDXTABLECOUNT)) \
		if (LoadTable(PtrM->SlaveID)) {

#define DISCARD_SLAVE(AbortOps) } else { AbortOps; }

#define START_FINDMASTER(AbortOps, AbortRetval, Key) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT ID = 0; \
	LFCoreAttributes* PtrM; \
	if (Tables[IDXTABLE_MASTER]->FindKey(Key, ID, (void*&)PtrM)) {

#define END_FINDMASTER() }

#define START_ITERATEMASTER(AbortOps, AbortRetval) \
	LFItemDescriptor* i = NULL; \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT ID = 0; \
	LFCoreAttributes* PtrM; \
	while (Tables[IDXTABLE_MASTER]->FindNext(ID, (void*&)PtrM)) {

#define END_ITERATEMASTER() }

#define START_ITERATEALL(AbortOps, AbortRetval) \
	UINT Type = LFTypeFile | slot->Source; \
	if (!IsStoreMounted(slot)) Type |= LFTypeNotMounted | LFTypeGhosted; \
	LOAD_MASTER(AbortOps, AbortRetval); \
	INT IDs[IDXTABLECOUNT]; ZeroMemory(IDs, sizeof(IDs)); \
	LFCoreAttributes* PtrM; \
	LFItemDescriptor* i = NULL; \
	while (Tables[IDXTABLE_MASTER]->FindNext(IDs[IDXTABLE_MASTER], (void*&)PtrM)) {

#define END_ITERATEALL() }

#define IN_TRANSACTIONLIST(tl) \
	UINT ItemID = 0; \
	for (; ItemID<tl->m_ItemCount; ItemID++) \
	{ \
		if ((strcmp(tl->m_Items[ItemID].StoreID, slot->StoreID)==0) && (strcmp(tl->m_Items[ItemID].FileID, PtrM->FileID)==0)) \
			goto Exists; \
	} \
	continue; \
	Exists: \
	i = tl->m_Items[ItemID].pItemDescriptor; \

#define DELETE_FILE() \
	UINT Result = LFOk; \
	if (PutInTrash) \
	{ \
		REMOVE_STATS(); \
		PtrM->Flags &= ~LFFlagNew; \
		if (!(PtrM->Flags & LFFlagTrash)) \
		{ \
			PtrM->Flags |= LFFlagTrash; \
			GetSystemTimeAsFileTime(&PtrM->DeleteTime); \
		} \
		Tables[IDXTABLE_MASTER]->MakeDirty(); \
		ADD_STATS(); \
	} \
	else \
	{ \
		if (!(PtrM->Flags & LFFlagLink)) \
			Result = DeletePhysicalFile(PtrM); \
		if (Result==LFOk) \
		{ \
			LOAD_SLAVE(); \
			Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]); \
			DISCARD_SLAVE(Result = LFIndexTableLoadError); \
		} \
		if (Result==LFOk) \
		{ \
			REMOVE_STATS(); \
			Tables[IDXTABLE_MASTER]->Invalidate(PtrM); \
		} \
	}


// CIndex
//

CIndex::CIndex(LFStoreDescriptor* _slot, BOOL ForMainIndex)
{
	assert(_slot);

	ZeroMemory(Tables, sizeof(Tables));
	slot = _slot;
	wcscpy_s(IdxPath, MAX_PATH, ForMainIndex ? slot->IdxPathMain : slot->IdxPathAux);
	TrackStats = ForMainIndex;
}

CIndex::~CIndex()
{
	for (UINT a=0; a<IDXTABLECOUNT; a++)
		if (Tables[a])
			delete Tables[a];
}

BOOL CIndex::LoadTable(UINT TableID, UINT* Result)
{
	assert(TableID<IDXTABLECOUNT);

	if (!Tables[TableID])
		Tables[TableID] = new CHeapfile(IdxPath, (BYTE)TableID);

	if (Result)
		*Result = Tables[TableID]->m_OpenStatus;

	return (Tables[TableID]->m_OpenStatus!=HeapError) && (Tables[TableID]->m_OpenStatus!=HeapNoAccess) && (Tables[TableID]->m_OpenStatus!=HeapCannotCreate);
}

BOOL CIndex::Create()
{
	BOOL Result = TRUE;

	if (IdxPath[0]!=L'\0')
		for (UINT a=0; a<IDXTABLECOUNT; a++)
			if (!LoadTable(a))
				Result = FALSE;

	return Result;
}

UINT CIndex::Check(BOOL Scheduled, BOOL* pRepaired, LFProgress* pProgress)
{
	#define COMPACT(idx) \
	if (!DirFreeSpace(IdxPath, Tables[idx]->GetRequiredFileSize())) \
		return LFNotEnoughFreeDiscSpace; \
	if (!Tables[idx]->Compact()) \
		return LFIndexRepairError;

	BOOL Writeable = DirWriteable(IdxPath);
	if (Scheduled && !Writeable)
		return LFDriveWriteProtected;

	BOOL SlaveReindex = FALSE;
	BOOL UpdateContexts = FALSE;
	UINT tRes[IDXTABLECOUNT];
	UINT RecordSize = 0;

	// Tabellen prüfen
	for (UINT a=0; a<IDXTABLECOUNT; a++)
	{
		LoadTable(a, &tRes[a]);
		switch (tRes[a])
		{
		case HeapCreated:
			if (a==IDXTABLE_MASTER)
				return LFIndexRepairError;	// TODO
			*pRepaired = SlaveReindex = TRUE;
			break;
		case HeapMaintenanceRequired:
			COMPACT(a);
			*pRepaired = UpdateContexts = TRUE;
			tRes[a] = Tables[a]->m_OpenStatus;
			break;

		case HeapNoAccess:
			return LFIndexAccessError;
		case HeapError:
			return LFIndexRepairError;
		case HeapCannotCreate:
			return Writeable ? LFIndexCreateError : LFDriveWriteProtected;
		}

		RecordSize += Tables[a]->GetRequiredElementSize();

		// Progress
		if (pProgress)
		{
			pProgress->MinorCurrent++;
			if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
				return LFCancel;
		}
	}

	// Enough space for slave reindexing if neccessary?
	if (SlaveReindex)
		if (!DirFreeSpace(IdxPath, RecordSize*Tables[IDXTABLE_MASTER]->GetItemCount()))
			return LFNotEnoughFreeDiscSpace;

	// Reset statistics
	ZeroMemory(slot->FileCount, sizeof(slot->FileCount));
	ZeroMemory(slot->FileSize, sizeof(slot->FileSize));

	// Traverse index
	INT ID = 0;
	LFCoreAttributes* PtrM;
	while (Tables[IDXTABLE_MASTER]->FindNext(ID, (void*&)PtrM))
	{
		// Operations below modify index
		if (!Writeable)
		{
			AddFileToStatistics(PtrM);
			continue;
		}

		WCHAR fn[2*MAX_PATH];
		if (Scheduled || SlaveReindex)
			GetFileLocation(slot->DatPath, PtrM, fn, 2*MAX_PATH);

		// File body present?
		if (Scheduled)
			if ((!(PtrM->Flags & LFFlagLink)) && IsStoreMounted(slot))
			{
				UINT Flags = FileExists(fn) ? 0 : LFFlagMissing;
				if ((Flags & LFFlagMissing)!=(PtrM->Flags & LFFlagMissing))
				{
					PtrM->Flags &= ~LFFlagMissing;
					PtrM->Flags |= Flags;
					Tables[IDXTABLE_MASTER]->MakeDirty();

					*pRepaired = TRUE;
				}
			}

		// Index version changed? Then update contexts!
		if (UpdateContexts)
		{
			SetFileContext(PtrM, TRUE);
			Tables[IDXTABLE_MASTER]->MakeDirty();
		}

		AddFileToStatistics(PtrM);

		// Slave index missing?
		if (SlaveReindex)
			if ((PtrM->SlaveID) && (PtrM->SlaveID<IDXTABLECOUNT))
				if (tRes[PtrM->SlaveID]==HeapCreated)
				{
					const UINT Type = LFTypeFile;
					LFItemDescriptor* i;
					BUILD_ITEMDESCRIPTOR();
					SetAttributesFromFile(i, &fn[4]);	// No fully qualified path allowed, skip prefix
					Tables[PtrM->SlaveID]->Add(i);
					LFFreeItemDescriptor(i);
				}
	}

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			return LFCancel;
	}

	// Compaction
	if (Scheduled)
		for (UINT a=0; a<IDXTABLECOUNT; a++)
		{
			COMPACT(a);

			*pRepaired |= (tRes[a]!=Tables[a]->m_OpenStatus);
			tRes[a] = Tables[a]->m_OpenStatus;

			// Progress
			if (pProgress)
			{
				pProgress->MinorCurrent++;
				if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
					return LFCancel;
			}
		}

	return LFOk;
}


// Physical files

UINT CIndex::RenamePhysicalFile(LFCoreAttributes* PtrM, WCHAR* NewName)
{
	if (!IsStoreMounted(slot))
		return LFStoreNotMounted;

	WCHAR Path1[2*MAX_PATH];
	GetFileLocation(slot->DatPath, PtrM, Path1, 2*MAX_PATH);

	WCHAR OldName[256];
	wcscpy_s(OldName, 256, PtrM->FileName);
	wcscpy_s(PtrM->FileName, 256, NewName);

	WCHAR Path2[2*MAX_PATH];
	GetFileLocation(slot->DatPath, PtrM, Path2, 2*MAX_PATH);

	if (FileExists(Path2))
		return LFOk;
	if (!FileExists(Path1))
	{
		wcscpy_s(PtrM->FileName, 256, OldName);
		return LFNoFileBody;
	}

	if (!MoveFile(Path1, Path2))
	{
		wcscpy_s(PtrM->FileName, 256, OldName);
		return LFCannotRenameFile;
	}

	return LFOk;
}

UINT CIndex::DeletePhysicalFile(LFCoreAttributes* PtrM)
{
	if (!IsStoreMounted(slot))
		return LFStoreNotMounted;

	WCHAR IdxPath[2*MAX_PATH];
	GetFileLocation(slot->DatPath, PtrM, IdxPath, 2*MAX_PATH);

	WCHAR* LastBackslash = wcsrchr(IdxPath, L'\\');
	if (LastBackslash)
		*(LastBackslash+1) = L'\0';

	if (RemoveDir(IdxPath))
		return LFOk;

	DWORD err = GetLastError();
	return (err==ERROR_NO_MORE_FILES) || (err==ERROR_FILE_NOT_FOUND) || (err==ERROR_PATH_NOT_FOUND) ? LFOk : LFCannotDeleteFile;
}


// Transactions

void CIndex::AddFileToStatistics(LFCoreAttributes* PtrM)
{
	assert(slot);

	#define ADD_FILE(Context) { slot->FileCount[Context]++; slot->FileSize[Context] += PtrM->FileSize; }
	COUNT_FILE(ADD_FILE);
}

void CIndex::RemoveFileFromStatistics(LFCoreAttributes* PtrM)
{
	assert(slot);

	#define SUB_FILE(Context) { slot->FileCount[Context]--; slot->FileSize[Context] -= PtrM->FileSize; }
	COUNT_FILE(SUB_FILE);
}

UINT CIndex::AddItem(LFItemDescriptor* i)
{
	assert(i);

	// Master
	if (!LoadTable(IDXTABLE_MASTER))
		return LFIndexTableLoadError;

	// Slave
	if ((i->CoreAttributes.SlaveID) && (i->CoreAttributes.SlaveID<IDXTABLECOUNT))
	{
		if (!LoadTable(i->CoreAttributes.SlaveID))
			return LFIndexTableLoadError;

		Tables[i->CoreAttributes.SlaveID]->Add(i);
	}

	Tables[IDXTABLE_MASTER]->Add(i);

	if (TrackStats)
		AddFileToStatistics(&i->CoreAttributes);

	return LFOk;
}

BOOL CIndex::UpdateSystemFlags(LFItemDescriptor* i, BOOL Exists, BOOL RemoveNew)
{
	assert(i);

	START_FINDMASTER(, Exists, i->CoreAttributes.FileID);

	if (!(i->CoreAttributes.Flags & LFFlagLink))
		if (Exists)
		{
			i->CoreAttributes.Flags &= ~LFFlagMissing;
		}
		else
		{
			i->CoreAttributes.Flags |= LFFlagMissing;
		}

	if (RemoveNew)
		i->CoreAttributes.Flags &= ~LFFlagNew;

	REMOVE_STATS();
	Tables[IDXTABLE_MASTER]->Update(i, PtrM);
	ADD_STATS();

	END_FINDMASTER();

	return !(i->CoreAttributes.Flags & LFFlagMissing);
}

void CIndex::Update(LFTransactionList* tl, LFVariantData* v1, LFVariantData* v2, LFVariantData* v3)
{
	assert(tl);
	assert(v1);

	START_ITERATEALL(tl->SetError(slot->StoreID, LFIndexTableLoadError),);
	IN_TRANSACTIONLIST(tl);
	REMOVE_STATS();

	// Attribute setzen
	i->CoreAttributes.Flags &= ~LFFlagNew;

	LFSetAttributeVariantData(i, *v1);
	BOOL IncludeSlave = (v1->Attr>LFLastCoreAttribute);
	if (v2)
	{
		LFSetAttributeVariantData(i, *v2);
		IncludeSlave |= (v2->Attr>LFLastCoreAttribute);
	}
	if (v3)
	{
		LFSetAttributeVariantData(i, *v3);
		IncludeSlave |= (v3->Attr>LFLastCoreAttribute);
	}

	// Phys. Datei umbenennen ?
	if (!(PtrM->Flags & LFFlagLink))
		if (wcscmp(i->CoreAttributes.FileName, PtrM->FileName)!=0)
		{
			UINT Result = RenamePhysicalFile(PtrM, i->CoreAttributes.FileName);
			switch (Result)
			{
			case LFOk:
				i->CoreAttributes.Flags &= ~LFFlagMissing;
				break;
			case LFNoFileBody:
				i->CoreAttributes.Flags |= LFFlagMissing;
			default:
				wcscpy_s(i->CoreAttributes.FileName, 256, PtrM->FileName);
				tl->m_Items[ItemID].LastError = tl->m_LastError = Result;
			}
		}

	// Master
	Tables[IDXTABLE_MASTER]->Update(i, PtrM);
	ADD_STATS();

	// Slave
	if (IncludeSlave)
	{
		LOAD_SLAVE();

		void* PtrS;
		if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
			Tables[PtrM->SlaveID]->Update(i, PtrS);

		DISCARD_SLAVE(tl->m_Items[ItemID].LastError = tl->m_LastError = LFIndexTableLoadError);
	}

	tl->m_Items[ItemID].Processed = tl->m_Modified = TRUE;

	END_ITERATEALL();

	// Invalid items
	tl->SetError(slot->StoreID, LFIllegalKey);
}

void CIndex::Archive(LFTransactionList* tl)
{
	assert(tl);

	START_ITERATEMASTER(tl->SetError(slot->StoreID, LFIndexTableLoadError),);
	IN_TRANSACTIONLIST(tl);
	REMOVE_STATS();

	PtrM->Flags &= ~LFFlagNew;
	if (!(PtrM->Flags & LFFlagArchive))
	{
		PtrM->Flags |= LFFlagArchive;
		GetSystemTimeAsFileTime(&PtrM->ArchiveTime);
	}

	Tables[IDXTABLE_MASTER]->MakeDirty();
	ADD_STATS();

	tl->m_Items[ItemID].Processed = tl->m_Modified = TRUE;

	END_ITERATEMASTER();

	// Invalid items
	tl->SetError(slot->StoreID, LFIllegalKey);
}

void CIndex::Delete(LFTransactionList* tl, BOOL PutInTrash, LFProgress* pProgress)
{
	assert(tl);

	START_ITERATEALL(tl->SetError(slot->StoreID, LFIndexTableLoadError),);
	IN_TRANSACTIONLIST(tl);

	// Progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, PtrM->FileName);
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
		{
			tl->m_LastError = LFCancel;
			return;
		}
	}

	DELETE_FILE();
	if (Result==LFOk)
		tl->m_Modified = TRUE;

	tl->SetError(ItemID, Result, pProgress);
	if (pProgress)
		if (pProgress->UserAbort)
			return;

	END_ITERATEALL();

	// Invalid items are perceived as deleted
	tl->SetError(slot->StoreID, LFOk);
}

void CIndex::ResolvePhysicalLocations(LFTransactionList* tl)
{
	assert(tl);

	if (!IsStoreMounted(slot))
	{
		tl->SetError(slot->StoreID, LFStoreNotMounted);
		return;
	}

	START_ITERATEMASTER(tl->SetError(slot->StoreID, LFIndexTableLoadError),);
	IN_TRANSACTIONLIST(tl);

	GetFileLocation(slot->DatPath, PtrM, tl->m_Items[ItemID].Path, 2*MAX_PATH);
	tl->m_Items[ItemID].Processed = TRUE;

	END_ITERATEMASTER();
}

void CIndex::Retrieve(LFFilter* f, LFSearchResult* Result)
{
	assert(f);
	assert(f->Mode>=LFFilterModeDirectoryTree);
	assert(Result);

	START_ITERATEALL(Result->m_LastError = LFIndexTableLoadError,);

	BOOL CheckSearchterm = FALSE;
	if (!PassesFilter(IDXTABLE_MASTER, PtrM, f, CheckSearchterm))
		continue;

	void* PtrS = NULL;

	if (!f->Options.IgnoreSlaves)
	{
		LOAD_SLAVE();

		if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
			if (!PassesFilter(PtrM->SlaveID, PtrS, f, CheckSearchterm))
				continue;

		DISCARD_SLAVE(Result->m_LastError = LFIndexTableLoadError);
	}

	BUILD_ITEMDESCRIPTOR();
	if (PtrS)
		AttachSlave(i, PtrM->SlaveID, PtrS);

	if (PassesFilter(i, f))
		if (Result->AddItem(i))
			continue;

	LFFreeItemDescriptor(i);

	END_ITERATEALL();
}

void CIndex::AddToSearchResult(LFTransactionList* il, LFSearchResult* Result)
{
	assert(il);
	assert(Result);

	START_ITERATEALL(il->SetError(slot->StoreID, LFIndexTableLoadError); Result->m_LastError = LFIndexTableLoadError, );
	IN_TRANSACTIONLIST(il);
	BUILD_ITEMDESCRIPTOR();

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE(Result->m_LastError = il->m_Items[ItemID].LastError = LFIndexTableLoadError);

	Result->AddItem(i);
	il->m_Items[ItemID].Processed = TRUE;

	END_ITERATEALL();
}

void CIndex::TransferTo(CIndex* idxDst1, CIndex* idxDst2, LFStoreDescriptor* slotDst, LFTransactionList* il, LFStoreDescriptor* slotSrc, BOOL move, LFProgress* pProgress)
{
	assert(il);

#define ABORT(r) { LFFreeItemDescriptor(i); \
	il->SetError(ItemID, r, pProgress); \
	if (pProgress) \
		if (pProgress->UserAbort) \
			return; \
	continue; }

	if (!IsStoreMounted(slot))
	{
		il->SetError(slot->StoreID, LFStoreNotMounted);
		return;
	}

	START_ITERATEALL(il->SetError(slot->StoreID, LFIndexTableLoadError), );
	IN_TRANSACTIONLIST(il);

	// Progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, PtrM->FileName);
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
		{
			il->m_LastError = LFCancel;
			return;
		}
	}

	BUILD_ITEMDESCRIPTOR();
	i->CoreAttributes.Flags |= LFFlagNew;

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE(ABORT(LFIndexTableLoadError));

	WCHAR PathSrc[2*MAX_PATH];
	GetFileLocation(slotSrc->DatPath, PtrM, PathSrc, 2*MAX_PATH);

	WCHAR PathDst[2*MAX_PATH];
	UINT Result = PrepareImport(slotDst, i, PathDst, 2*MAX_PATH);
	if (Result!=LFOk)
		ABORT(Result);

	// Files with "link" flag do not posses a file body
	if (!(PtrM->Flags & LFFlagLink))
		if (!(CopyFile(PathSrc, PathDst, FALSE)))
		{
			WCHAR* LastBackslash = wcsrchr(IdxPath, L'\\');
			if (LastBackslash)
				*(LastBackslash+1) = L'\0';

			RemoveDir(IdxPath);
			ABORT(LFCannotImportFile);
		}

	if (idxDst1)
		Result = idxDst1->AddItem(i);
	if ((idxDst2) && (Result==LFOk))
		Result |= idxDst2->AddItem(i);

	if (Result!=LFOk)
		ABORT(Result);

	if (move)
	{
		// Files with "link" flag do not posses a file body
		if (!(PtrM->Flags & LFFlagLink))
		{
			UINT Result = DeletePhysicalFile(PtrM);
			if (Result!=LFOk)
				ABORT(Result);
		}

		if ((PtrM->SlaveID) && (PtrM->SlaveID<IDXTABLECOUNT))
			Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);

		Tables[IDXTABLE_MASTER]->Invalidate(PtrM);
	}

	ABORT(LFOk);

	END_ITERATEALL();

	// Invalid items
	il->SetError(slot->StoreID, LFIllegalKey, pProgress);
}
