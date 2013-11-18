
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
	LFItemDescriptor* i = LFAllocItemDescriptor(); \
	i->Type = LFTypeFile | slot->Source; \
	if (!IsStoreMounted(slot)) i->Type |= LFTypeNotMounted | LFTypeGhosted; \
	strcpy_s(i->StoreID, LFKeySize, slot->StoreID); \
	Tables[IDMaster]->WriteToItemDescriptor(i, PtrM);

#define APPEND_ITEMDESCRIPTOR() \
	void* PtrS; \
	if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS)) \
		Tables[PtrM->SlaveID]->WriteToItemDescriptor(i, PtrS);

#define LOAD_MASTER(AbortOps, AbortRetval) \
	if (!LoadTable(IDMaster)) { AbortOps; return AbortRetval; }

#define LOAD_SLAVE() \
	if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount)) \
		if (LoadTable(PtrM->SlaveID)) {

#define DISCARD_SLAVE(AbortOps) } else { AbortOps; }

#define START_FINDMASTER(AbortOps, AbortRetval, Key) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	int ID = 0; \
	LFCoreAttributes* PtrM; \
	if (Tables[IDMaster]->FindKey(Key, ID, (void*&)PtrM)) {

#define END_FINDMASTER() }

#define START_ITERATEMASTER(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	int ID = 0; \
	LFCoreAttributes* PtrM; \
	while (Tables[IDMaster]->FindNext(ID, (void*&)PtrM)) {

#define END_ITERATEMASTER() }

#define START_ITERATEALL(AbortOps, AbortRetval) \
	LOAD_MASTER(AbortOps, AbortRetval); \
	int IDs[IdxTableCount]; ZeroMemory(IDs, sizeof(IDs)); \
	LFCoreAttributes* PtrM; \
	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM)) {

#define END_ITERATEALL() }

#define IN_FILEIDLIST(il) \
	unsigned int ItemID = 0; \
	for (; ItemID<il->m_ItemCount; ItemID++) \
		if ((strcmp(il->m_Items[ItemID].StoreID, slot->StoreID)==0) && (strcmp(il->m_Items[ItemID].FileID, PtrM->FileID)==0)) \
			goto Exists; \
	continue; \
	Exists:

#define IN_TRANSACTIONLIST(tl) \
	unsigned int ItemID = 0; \
	LFItemDescriptor* i; \
	for (; ItemID<tl->m_ItemCount; ItemID++) { \
		i = tl->m_Items[ItemID].Item; \
		if ((i->Type & LFTypeMask)==LFTypeFile) \
			if ((strcmp(i->StoreID, slot->StoreID)==0) && (strcmp(i->CoreAttributes.FileID, PtrM->FileID)==0)) \
				goto Exists; \
	} \
	continue; \
	Exists:

#define DELETE_FILE() \
	unsigned int res = LFOk; \
	if (PutInTrash) \
	{ \
		REMOVE_STATS(); \
		PtrM->Flags &= ~LFFlagNew; \
		if (!(PtrM->Flags & LFFlagTrash)) \
		{ \
			PtrM->Flags |= LFFlagTrash; \
			GetSystemTimeAsFileTime(&PtrM->DeleteTime); \
		} \
		Tables[IDMaster]->MakeDirty(); \
		ADD_STATS(); \
	} \
	else \
	{ \
		if (!(PtrM->Flags & LFFlagLink)) \
			res = DeletePhysicalFile(PtrM); \
		if (res==LFOk) \
		{ \
			LOAD_SLAVE(); \
			Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]); \
			DISCARD_SLAVE(res = LFIndexTableLoadError); \
		} \
		if (res==LFOk) \
		{ \
			REMOVE_STATS(); \
			Tables[IDMaster]->Invalidate(PtrM); \
		} \
	}


// CIndex
//

CIndex::CIndex(LFStoreDescriptor* _slot, bool ForMainIndex)
{
	assert(_slot);

	ZeroMemory(Tables, sizeof(Tables));
	slot = _slot;
	wcscpy_s(IdxPath, MAX_PATH, ForMainIndex ? slot->IdxPathMain : slot->IdxPathAux);
	TrackStats = ForMainIndex;
}

CIndex::~CIndex()
{
	for (unsigned int a=0; a<IdxTableCount; a++)
		if (Tables[a])
			delete Tables[a];
}

bool CIndex::LoadTable(unsigned int ID, unsigned int* res)
{
	assert(ID<IdxTableCount);

	if (!Tables[ID])
		switch (ID)
		{
		case IDMaster:
			Tables[ID] = new CIdxTableMaster(IdxPath);
			break;
		case IDSlaveDocuments:
			Tables[ID] = new CIdxTableDocuments(IdxPath);
			break;
		case IDSlaveMessages:
			Tables[ID] = new CIdxTableMessages(IdxPath);
			break;
		case IDSlaveAudio:
			Tables[ID] = new CIdxTableAudio(IdxPath);
			break;
		case IDSlavePictures:
			Tables[ID] = new CIdxTablePictures(IdxPath);
			break;
		case IDSlaveVideos:
			Tables[ID] = new CIdxTableVideos(IdxPath);
			break;
		default:
			assert(false);

			if (res)
				*res = HeapError;
			return false;
		}

	if (res)
		*res = Tables[ID]->OpenStatus;

	return (Tables[ID]->OpenStatus!=HeapError) && (Tables[ID]->OpenStatus!=HeapNoAccess) && (Tables[ID]->OpenStatus!=HeapCannotCreate);
}

bool CIndex::Create()
{
	bool res = true;

	if (IdxPath[0]!=L'\0')
		for (unsigned int a=0; a<IdxTableCount; a++)
			if (!LoadTable(a))
				res = false;

	return res;
}

unsigned int CIndex::Check(bool Scheduled, bool* pRepaired, LFProgress* pProgress)
{
	#define COMPACT(idx) \
	if (!DirFreeSpace(IdxPath, Tables[idx]->GetRequiredDiscSize())) \
		return LFNotEnoughFreeDiscSpace; \
	if (!Tables[idx]->Compact()) \
		return LFIndexRepairError;

	bool Writeable = DirWriteable(IdxPath);
	if (Scheduled && !Writeable)
		return LFDriveWriteProtected;

	bool SlaveReindex = false;
	bool UpdateContexts = false;
	unsigned int tRes[IdxTableCount];
	unsigned int RecordSize = 0;

	// Tabellen prüfen
	for (unsigned int a=0; a<IdxTableCount; a++)
	{
		LoadTable(a, &tRes[a]);
		switch (tRes[a])
		{
		case HeapNoAccess:
			return LFIndexAccessError;
		case HeapError:
			return LFIndexRepairError;
		case HeapCannotCreate:
			return Writeable ? LFIndexCreateError : LFDriveWriteProtected;
		case HeapCreated:
			if (a==IDMaster)
				return LFIndexRepairError;	// TODO
			*pRepaired = SlaveReindex = true;
			break;
		case HeapMaintenanceRequired:
			COMPACT(a);
			*pRepaired = UpdateContexts = true;
			tRes[a] = Tables[a]->OpenStatus;
			break;
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
		if (!DirFreeSpace(IdxPath, RecordSize*Tables[IDMaster]->GetItemCount()))
			return LFNotEnoughFreeDiscSpace;

	// Reset statistics
	ZeroMemory(slot->FileCount, sizeof(slot->FileCount)); \
	ZeroMemory(slot->FileSize, sizeof(slot->FileSize));

	// Traverse index
	int ID = 0;
	LFCoreAttributes* PtrM;
	while (Tables[IDMaster]->FindNext(ID, (void*&)PtrM))
	{
		AddFileToStatistics(PtrM);

		// Operations below modify index
		if (!Writeable)
			continue;

		wchar_t fn[2*MAX_PATH];
		if (Scheduled || SlaveReindex)
			GetFileLocation(slot->DatPath, PtrM, fn, 2*MAX_PATH);

		// File body present?
		if (Scheduled)
			if ((!(PtrM->Flags & LFFlagLink)) && IsStoreMounted(slot))
			{
				unsigned int Flags = FileExists(fn) ? 0 : LFFlagMissing;
				if ((Flags & LFFlagMissing)!=(PtrM->Flags & LFFlagMissing))
				{
					PtrM->Flags &= ~LFFlagMissing;
					PtrM->Flags |= Flags;
					Tables[IDMaster]->MakeDirty();

					*pRepaired = true;
				}
			}

		// Index version changed? Then update contexts!
		if (UpdateContexts)
		{
			SetFileContext(PtrM, true);
			Tables[IDMaster]->MakeDirty();
		}

		// Slave index missing?
		if (SlaveReindex)
			if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
				if (tRes[PtrM->SlaveID]==HeapCreated)
				{
					BUILD_ITEMDESCRIPTOR();
					SetAttributesFromFile(i, &fn[4]);	// No fully qualified path allowed, skip prefix
					Tables[PtrM->SlaveID]->Add(i);
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
		for (unsigned int a=0; a<IdxTableCount; a++)
		{
			COMPACT(a);

			*pRepaired |= (tRes[a]!=Tables[a]->OpenStatus);
			tRes[a] = Tables[a]->OpenStatus;

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

unsigned int CIndex::RenamePhysicalFile(LFCoreAttributes* PtrM, wchar_t* NewName)
{
	if (!IsStoreMounted(slot))
		return LFStoreNotMounted;

	wchar_t Path1[2*MAX_PATH];
	GetFileLocation(slot->DatPath, PtrM, Path1, 2*MAX_PATH);

	wchar_t OldName[256];
	wcscpy_s(OldName, 256, PtrM->FileName);
	wcscpy_s(PtrM->FileName, 256, NewName);

	wchar_t Path2[2*MAX_PATH];
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

unsigned int CIndex::DeletePhysicalFile(LFCoreAttributes* PtrM)
{
	if (!IsStoreMounted(slot))
		return LFStoreNotMounted;

	wchar_t IdxPath[2*MAX_PATH];
	GetFileLocation(slot->DatPath, PtrM, IdxPath, 2*MAX_PATH);

	wchar_t* LastBackslash = wcsrchr(IdxPath, L'\\');
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

unsigned int CIndex::AddItem(LFItemDescriptor* i)
{
	assert(i);

	// Master
	if (!LoadTable(IDMaster))
		return LFIndexTableLoadError;

	// Slave
	if ((i->CoreAttributes.SlaveID) && (i->CoreAttributes.SlaveID<IdxTableCount))
	{
		if (!LoadTable(i->CoreAttributes.SlaveID))
			return LFIndexTableLoadError;

		Tables[i->CoreAttributes.SlaveID]->Add(i);
	}

	Tables[IDMaster]->Add(i);

	if (TrackStats)
		AddFileToStatistics(&i->CoreAttributes);

	return LFOk;
}

bool CIndex::UpdateSystemFlags(LFItemDescriptor* i, bool Exists, bool RemoveNew)
{
	assert(i);

	START_FINDMASTER(, Exists, i->CoreAttributes.FileID);

	if (!(i->CoreAttributes.Flags & LFFlagLink))
		if (Exists)
		{
			i->CoreAttributes.Flags &= LFFlagMissing;
		}
		else
		{
			i->CoreAttributes.Flags |= LFFlagMissing;
		}

	if (RemoveNew)
		i->CoreAttributes.Flags &= ~LFFlagNew;

	REMOVE_STATS();
	Tables[IDMaster]->Update(i, PtrM);
	ADD_STATS();

	END_FINDMASTER();
	return !(i->CoreAttributes.Flags & LFFlagMissing);
}

void CIndex::Update(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	assert(tl);

	START_ITERATEALL(tl->SetError(slot->StoreID, LFIndexTableLoadError),);
	IN_TRANSACTIONLIST(tl);
	REMOVE_STATS();

	// Attribute setzen
	i->CoreAttributes.Flags &= ~LFFlagNew;

	LFSetAttributeVariantData(i, value1);
	bool IncludeSlave = (value1->Attr>LFLastCoreAttribute);
	if (value2)
	{
		LFSetAttributeVariantData(i, value2);
		IncludeSlave |= (value2->Attr>LFLastCoreAttribute);
	}
	if (value3)
	{
		LFSetAttributeVariantData(i, value3);
		IncludeSlave |= (value3->Attr>LFLastCoreAttribute);
	}

	// Phys. Datei umbenennen ?
	if (!(PtrM->Flags & LFFlagLink))
		if (wcscmp(i->CoreAttributes.FileName, PtrM->FileName)!=0)
		{
			unsigned int res = RenamePhysicalFile(PtrM, i->CoreAttributes.FileName);
			switch (res)
			{
			case LFOk:
				i->CoreAttributes.Flags &= ~LFFlagMissing;
				break;
			case LFNoFileBody:
				i->CoreAttributes.Flags |= LFFlagMissing;
			default:
				wcscpy_s(i->CoreAttributes.FileName, 256, PtrM->FileName);
				tl->m_Items[ItemID].LastError = tl->m_LastError = res;
			}
		}

	// Master
	Tables[IDMaster]->Update(i, PtrM);
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

	tl->m_Items[ItemID].Processed = tl->m_Changes = true;

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

	Tables[IDMaster]->MakeDirty();
	ADD_STATS();

	tl->m_Items[ItemID].Processed = tl->m_Changes = true;

	END_ITERATEMASTER();

	// Invalid items
	tl->SetError(slot->StoreID, LFIllegalKey);
}

void CIndex::Delete(LFTransactionList* tl, bool PutInTrash, LFProgress* pProgress)
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
	if (res==LFOk)
		tl->m_Changes = true;

	tl->SetError(ItemID, res, pProgress);
	if (pProgress)
		if (pProgress->UserAbort)
			return;

	END_ITERATEALL();

	// Invalid items are perceived as deleted
	tl->SetError(slot->StoreID, LFOk);
}

void CIndex::Delete(LFFileIDList* il, bool PutInTrash, LFProgress* pProgress)
{
	assert(il);

	START_ITERATEALL(il->SetError(slot->StoreID, LFIndexTableLoadError),);
	IN_FILEIDLIST(il);

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

	DELETE_FILE();

	il->SetError(ItemID, res, pProgress);
	if (pProgress)
		if (pProgress->UserAbort)
			return;

	END_ITERATEALL();

	// Invalid items are perceived as deleted
	il->SetError(slot->StoreID, LFOk);
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
	tl->m_Items[ItemID].Processed = true;

	END_ITERATEMASTER();
}

unsigned int CIndex::Rename(char* FileID, wchar_t* NewName)
{
	assert(FileID);
	assert(NewName);

	START_FINDMASTER(, LFIndexRepairError, FileID);
	REMOVE_STATS();

	unsigned int res = LFOk;
	if (PtrM->Flags & LFFlagLink)
	{
		wcscpy_s(PtrM->FileName, 256, NewName);
		PtrM->Flags &= ~(LFFlagMissing | LFFlagNew);
	}
	else
	{
		res = RenamePhysicalFile(PtrM, NewName);
		switch (res)
		{
		case LFOk:
			PtrM->Flags &= ~LFFlagMissing;
			break;
		case LFNoFileBody:
			PtrM->Flags |= LFFlagMissing;
		}

		PtrM->Flags &= ~LFFlagNew;
	}

	Tables[IDMaster]->MakeDirty();

	ADD_STATS();
	return res;

	END_FINDMASTER();
	return LFIllegalKey;
}

void CIndex::Retrieve(LFFilter* f, LFSearchResult* res)
{
	assert(f);
	assert(f->Mode>=LFFilterModeDirectoryTree);
	assert(res);

	START_ITERATEALL(res->m_LastError = LFIndexTableLoadError,);

	int pass = PassesFilterCore(PtrM, f);
	if (pass==-1)
		continue;

	BUILD_ITEMDESCRIPTOR();

	if (!f->Options.IgnoreSlaves)
	{
		LOAD_SLAVE();
		APPEND_ITEMDESCRIPTOR();
		DISCARD_SLAVE(res->m_LastError = LFIndexTableLoadError);

		if (!pass)
			pass = PassesFilterSlaves(i, f) ? 1 : -1;
	}

	if (pass!=-1)
		if (res->AddItemDescriptor(i))
			continue;

	// Discard
	LFFreeItemDescriptor(i);

	END_ITERATEALL();
}

void CIndex::AddToSearchResult(LFFileIDList* il, LFSearchResult* res)
{
	assert(il);
	assert(res);

	START_ITERATEALL(il->SetError(slot->StoreID, LFIndexTableLoadError); res->m_LastError = LFIndexTableLoadError, );
	IN_FILEIDLIST(il);
	BUILD_ITEMDESCRIPTOR();

	LOAD_SLAVE()
	APPEND_ITEMDESCRIPTOR();
	DISCARD_SLAVE(res->m_LastError = il->m_Items[ItemID].LastError = LFIndexTableLoadError);

	res->AddItemDescriptor(i);
	il->m_Items[ItemID].Processed = true;

	END_ITERATEALL();
}

void CIndex::TransferTo(CIndex* idxDst1, CIndex* idxDst2, LFStoreDescriptor* slotDst, LFFileIDList* il, LFStoreDescriptor* slotSrc, bool move, LFProgress* pProgress)
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
	IN_FILEIDLIST(il);

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

	wchar_t PathSrc[2*MAX_PATH];
	GetFileLocation(slotSrc->DatPath, PtrM, PathSrc, 2*MAX_PATH);

	wchar_t PathDst[2*MAX_PATH];
	unsigned int res = PrepareImport(slotDst, i, PathDst, 2*MAX_PATH);
	if (res!=LFOk)
		ABORT(res);

	// Files with "link" flag do not posses a file body
	if (!(PtrM->Flags & LFFlagLink))
		if (!(CopyFile(PathSrc, PathDst, FALSE)))
		{
			wchar_t* LastBackslash = wcsrchr(IdxPath, L'\\');
			if (LastBackslash)
				*(LastBackslash+1) = L'\0';

			RemoveDir(IdxPath);
			ABORT(LFCannotImportFile);
		}

	if (idxDst1)
		res = idxDst1->AddItem(i);
	if ((idxDst2) && (res==LFOk))
		res |= idxDst2->AddItem(i);

	if (res!=LFOk)
		ABORT(res);

	if (move)
	{
		// Files with "link" flag do not posses a file body
		if (!(PtrM->Flags & LFFlagLink))
		{
			unsigned int res = DeletePhysicalFile(PtrM);
			if (res!=LFOk)
				ABORT(res);
		}

		if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
			Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);

		Tables[IDMaster]->Invalidate(PtrM);
	}

	ABORT(LFOk);

	END_ITERATEALL();

	// Invalid items
	il->SetError(slot->StoreID, LFIllegalKey, pProgress);
}
