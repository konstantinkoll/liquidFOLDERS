
#include "stdafx.h"
#include "CIndex.h"
#include "LFItemDescriptor.h"
#include "Query.h"
#include "Stores.h"
#include <assert.h>


CIndex::CIndex(wchar_t* _Path, char* _StoreID, wchar_t* _DatPath)
{
	wcscpy_s(Path, MAX_PATH, _Path);
	strcpy_s(StoreID, LFKeySize, _StoreID);
	wcscpy_s(DatPath, MAX_PATH, _DatPath);
	ZeroMemory(Tables, sizeof(Tables));
}

CIndex::~CIndex()
{
	for (unsigned int a=0; a<IdxTableCount; a++)
		if (Tables[a])
			delete Tables[a];
}

__forceinline bool CIndex::LoadTable(unsigned int ID, unsigned int* res)
{
	assert(ID<IdxTableCount);

	if (!Tables[ID])
		switch (ID)
		{
		case IDMaster:
			Tables[ID] = new CIdxTableMaster(Path, L"Master.idx");
			break;
		case IDSlaveDocuments:
			Tables[ID] = new CIdxTableDocuments(Path, L"Docs.idx");
			break;
		case IDSlaveMails:
			Tables[ID] = new CIdxTableMails(Path, L"Mails.idx");
			break;
		case IDSlaveAudio:
			Tables[ID] = new CIdxTableAudio(Path, L"Audio.idx");
			break;
		case IDSlavePictures:
			Tables[ID] = new CIdxTablePictures(Path, L"Pictures.idx");
			break;
		case IDSlaveVideos:
			Tables[ID] = new CIdxTableVideos(Path, L"Videos.idx");
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

	for (unsigned int a=0; a<IdxTableCount; a++)
		if (!LoadTable(a))
			res = false;

	return res;
}

unsigned int CIndex::Check(bool scheduled)
{
	bool SlaveReindex = false;
	bool Repaired = false;
	unsigned int tres[IdxTableCount];
	unsigned int RecordSize = 0;

	// Tabellen prüfen
	for (unsigned int a=0; a<IdxTableCount; a++)
	{
		LoadTable(a, &tres[a]);
		switch (tres[a])
		{
		case HeapNoAccess:
			return IndexNoAccess;
		case HeapError:
			return IndexError;
		case HeapCannotCreate:
			return IndexCannotCreate;
		case HeapCreated:
			if (a==IDMaster)
				return IndexCompleteReindexRequired;
			SlaveReindex = true;
			Repaired = true;
			break;
		case HeapMaintenanceRequired:
			if (!Tables[a]->Compact())
				return IndexError;
			Repaired = true;
			tres[a] = Tables[a]->OpenStatus;
			break;
		}

		RecordSize += Tables[a]->GetRequiredElementSize();
	}

	// Index-Durchlauf
	if (scheduled || SlaveReindex)
	{
		if (!DirFreeSpace(Path, RecordSize*Tables[IDMaster]->GetItemCount()))
			return IndexNotEnoughFreeDiscSpace;

		int ID = 0;
		LFCoreAttributes* PtrM;

		while (Tables[IDMaster]->FindNext(ID, (void*&)PtrM))
		{
			// Ist der Dateikörper noch vorhanden?
			if ((!(PtrM->Flags & LFFlagLink)) && (DatPath[0]!='\0'))
			{
				wchar_t FilePath[2*MAX_PATH];
				GetFileLocation(DatPath, PtrM, FilePath, 2*MAX_PATH);

				unsigned int Flags = FileExists(FilePath) ? 0 : LFFlagMissing;
				if ((Flags & LFFlagMissing)!=(PtrM->Flags & LFFlagMissing))
				{
					PtrM->Flags &= ~LFFlagMissing;
					PtrM->Flags |= Flags;
					Tables[IDMaster]->MakeDirty();
				}
			}

			// TODO: registrierte Formate anpassen, ggf. neue Slaves erstellen
		}
	}

	// Kompaktieren
	if (scheduled)
		for (unsigned int a=0; a<IdxTableCount; a++)
		{
			if (!DirFreeSpace(Path, Tables[a]->GetRequiredDiscSize()))
				return IndexNotEnoughFreeDiscSpace;
			if (!Tables[a]->Compact())
				return IndexError;

			Repaired |= (tres[a]!=Tables[a]->OpenStatus);
			tres[a] = Tables[a]->OpenStatus;
		}

	// Ergebnis
	if (Repaired)
		for (unsigned int a=0; a<IdxTableCount; a++)
			if (tres[a]==HeapMaintenanceRecommended)
				return IndexPartiallyRepaired;

	return Repaired ? IndexFullyRepaired : IndexOk;
}

void CIndex::AddItem(LFItemDescriptor* i)
{
	assert(i);

	// Master
	LoadTable(IDMaster);
	Tables[IDMaster]->Add(i);

	// Slave
	if ((i->CoreAttributes.SlaveID) && (i->CoreAttributes.SlaveID<IdxTableCount))
	{
		LoadTable(i->CoreAttributes.SlaveID);
		Tables[i->CoreAttributes.SlaveID]->Add(i);
	}
}

unsigned int CIndex::RenamePhysicalFile(LFCoreAttributes* PtrM, wchar_t* NewName)
{
	if (!DatPath)
		return LFStoreNotMounted;
	if (DatPath[0]=='\0')
		return LFStoreNotMounted;

	wchar_t Path1[2*MAX_PATH];
	GetFileLocation(DatPath, PtrM, Path1, 2*MAX_PATH);

	wchar_t OldName[256];
	wcscpy_s(OldName, 256, PtrM->FileName);
	wcscpy_s(PtrM->FileName, 256, NewName);

	wchar_t Path2[2*MAX_PATH];
	GetFileLocation(DatPath, PtrM, Path2, 2*MAX_PATH);

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

void CIndex::Update(LFItemDescriptor* i, bool IncludeSlaves)
{
	assert(i);

	i->CoreAttributes.Flags &= ~LFFlagNew;

	// Master
	if (!LoadTable(IDMaster))
		return;

	// Items aktualisieren
	int ID = 0;
	LFCoreAttributes* PtrM;

	if (Tables[IDMaster]->FindKey(i->CoreAttributes.FileID, ID, (void*&)PtrM))
	{
		// Phys. Datei umbenennen ?
		if (!(PtrM->Flags & LFFlagLink))
			if (wcscmp(i->CoreAttributes.FileName, PtrM->FileName)!=0)
				switch (RenamePhysicalFile(PtrM, i->CoreAttributes.FileName))
				{
				case LFOk:
					i->CoreAttributes.Flags &= ~LFFlagMissing;
					break;
				case LFNoFileBody:
					i->CoreAttributes.Flags |= LFFlagMissing;
				default:
					wcscpy_s(i->CoreAttributes.FileName, 256, PtrM->FileName);
				}

		Tables[IDMaster]->Update(i, PtrM);
	}

	// Slave
	if ((i->CoreAttributes.SlaveID) && (i->CoreAttributes.SlaveID<IdxTableCount) && (IncludeSlaves))
		if (LoadTable(i->CoreAttributes.SlaveID))
			Tables[i->CoreAttributes.SlaveID]->Update(i);
}

bool CIndex::UpdateMissing(LFItemDescriptor* i, bool Exists)
{
	assert(i);

	if (i->CoreAttributes.Flags & LFFlagLink)
		return true;

	// Master
	if (!LoadTable(IDMaster))
		return Exists;

	// Items aktualisieren
	int ID = 0;
	LFCoreAttributes* PtrM;

	if (Tables[IDMaster]->FindKey(i->CoreAttributes.FileID, ID, (void*&)PtrM))
	{
		if (Exists)
		{
			if (PtrM->Flags & (LFFlagNew | LFFlagMissing))
			{
				i->CoreAttributes.Flags &= ~(LFFlagNew | LFFlagMissing);
				Tables[IDMaster]->Update(i, PtrM);
			}
		}
		else
		{
			if ((PtrM->Flags & LFFlagMissing)==0)
			{
				i->CoreAttributes.Flags |= LFFlagMissing;
				Tables[IDMaster]->Update(i, PtrM);
			}
		}
	}

	return !(i->CoreAttributes.Flags & LFFlagMissing);
}

void CIndex::Update(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	assert(tl);

	if (!LoadTable(IDMaster))
	{
		tl->SetError(StoreID, LFIndexTableLoadError);
		return;
	}

	// Items aktualisieren
	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		for (unsigned int a=0; a<tl->m_ItemCount; a++)
		{
			LFItemDescriptor* i = tl->m_Items[a].Item;
			if ((i->Type & LFTypeMask)==LFTypeFile)
				if ((strcmp(i->StoreID, StoreID)==0) && (strcmp(i->CoreAttributes.FileID, PtrM->FileID)==0))
				{
					// Attribute setzen
					i->CoreAttributes.Flags &= ~LFFlagNew;
					tl->m_Changes = true;

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
								tl->m_Items[a].LastError = tl->m_LastError = res;
							}
						}

					// Master
					Tables[IDMaster]->Update(i, PtrM);

					// Slave
					if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount) && (IncludeSlave))
						if (LoadTable(PtrM->SlaveID))
						{
							void* PtrS;

							if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
								Tables[PtrM->SlaveID]->Update(i, PtrS);
						}
						else
						{
							tl->m_Items[a].LastError = tl->m_LastError = LFIndexTableLoadError;
						}

					tl->m_Items[a].Processed = true;
				}
		}
	}

	// Ungültige Items finden
	tl->SetError(StoreID, LFIllegalKey);
}

unsigned int CIndex::DeletePhysicalFile(LFCoreAttributes* PtrM)
{
	if (!DatPath)
		return LFStoreNotMounted;
	if (DatPath[0]=='\0')
		return LFStoreNotMounted;

	wchar_t Path[2*MAX_PATH];
	GetFileLocation(DatPath, PtrM, Path, 2*MAX_PATH);

	wchar_t* LastBackslash = wcsrchr(Path, L'\\');
	if (LastBackslash)
		*(LastBackslash+1) = L'\0';

	return RemoveDir(Path) ? LFOk : LFCannotDeleteFile;
}

void CIndex::Delete(LFFileIDList* il, bool PutInTrash)
{
	assert(il);

	if (!LoadTable(IDMaster))
	{
		il->SetError(StoreID, LFIndexTableLoadError);
		return;
	}

	// Items löschen
	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		for (unsigned int a=0; a<il->m_ItemCount; a++)
			if ((strcmp(il->m_Items[a].StoreID, StoreID)==0) && (strcmp(il->m_Items[a].FileID, PtrM->FileID)==0))
			{
				if (PutInTrash)
				{
					PtrM->Flags |= LFFlagTrash;
					GetSystemTimeAsFileTime(&PtrM->DeleteTime);
					Tables[IDMaster]->MakeDirty();
				}
				else
				{
					// Files with "link" flag do not posses a file body
					if (!(PtrM->Flags & LFFlagLink))
					{
						unsigned int res = DeletePhysicalFile(PtrM);
						if (res!=LFOk)
							il->m_Items[a].LastError = il->m_LastError = res;
					}

					// Slave
					if (il->m_Items[a].LastError==LFOk)
						if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
							if (LoadTable(PtrM->SlaveID))
							{
								Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);
							}
							else
							{
								il->m_Items[a].LastError = il->m_LastError = LFIndexTableLoadError;
							}

					// Master
					if (il->m_Items[a].LastError==LFOk)
						Tables[IDMaster]->Invalidate(PtrM);
				}

				il->m_Items[a].Processed = true;
			}
	}

	// Ungültige Items finden
	il->SetError(StoreID, LFIllegalKey);
}

void CIndex::Delete(LFTransactionList* tl, bool PutInTrash)
{
	assert(tl);

	if (!LoadTable(IDMaster))
	{
		tl->SetError(StoreID, LFIndexTableLoadError);
		return;
	}

	// Items löschen
	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		for (unsigned int a=0; a<tl->m_ItemCount; a++)
		{
			LFItemDescriptor* i = tl->m_Items[a].Item;
			if ((i->Type & LFTypeMask)==LFTypeFile)
				if ((strcmp(i->StoreID, StoreID)==0) && (strcmp(i->CoreAttributes.FileID, PtrM->FileID)==0))
				{
					if (PutInTrash)
					{
						PtrM->Flags |= LFFlagTrash;
						GetSystemTimeAsFileTime(&PtrM->DeleteTime);
						Tables[IDMaster]->MakeDirty();
					}
					else
					{
						// Files with "link" flag do not posses a file body
						if (!(PtrM->Flags & LFFlagLink))
						{
							unsigned int res = DeletePhysicalFile(PtrM);
							if (res!=LFOk)
								tl->m_Items[a].LastError = tl->m_LastError = res;
						}

						// Slave
						if (tl->m_Items[a].LastError==LFOk)
							if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
								if (LoadTable(PtrM->SlaveID))
								{
									Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);
								}
								else
								{
									tl->m_Items[a].LastError = tl->m_LastError = LFIndexTableLoadError;
								}

						// Master
						if (tl->m_Items[a].LastError==LFOk)
						{
							Tables[IDMaster]->Invalidate(PtrM);
							tl->m_Changes = true;
						}

						tl->m_Items[a].Processed = true;
					}
				}
		}
	}

	// Ungültige Items finden
	tl->SetError(StoreID, LFIllegalKey);
}

void CIndex::ResolvePhysicalLocations(LFTransactionList* tl)
{
	bool Mounted = (DatPath!=NULL);
	if (DatPath)
		Mounted &= (DatPath[0]!='\0');

	if (!Mounted)
	{
		tl->SetError(StoreID, LFStoreNotMounted);
		return;
	}

	if (!LoadTable(IDMaster))
	{
		tl->SetError(StoreID, LFIndexTableLoadError);
		return;
	}

	int ID = 0;
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(ID, (void*&)PtrM))
		for (unsigned int a=0; a<tl->m_ItemCount; a++)
		{
			LFItemDescriptor* i = tl->m_Items[a].Item;
			if ((i->Type & LFTypeMask)==LFTypeFile)
				if ((strcmp(i->StoreID, StoreID)==0) && (strcmp(i->CoreAttributes.FileID, PtrM->FileID)==0))
				{
					GetFileLocation(DatPath, PtrM, tl->m_Items[a].Path, 2*MAX_PATH);
					tl->m_Items[a].Processed = true;
				}
		}
}

unsigned int CIndex::Rename(char* FileID, wchar_t* NewName)
{
	assert(FileID);
	assert(NewName);

	if (!LoadTable(IDMaster))
		return LFIndexRepairError;

	// Items aktualisieren
	int ID = 0;
	LFCoreAttributes* PtrM;

	if (Tables[IDMaster]->FindKey(FileID, ID, (void*&)PtrM))
	{
		PtrM->Flags &= ~LFFlagNew;

		unsigned int res = LFOk;
		if (!(PtrM->Flags & LFFlagLink))
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
		}
		else
		{
			wcscpy_s(PtrM->FileName, 256, NewName);
			PtrM->Flags &= ~LFFlagMissing;
		}

		Tables[IDMaster]->MakeDirty();
		return res;
	}

	return LFIllegalKey;
}

void CIndex::Retrieve(LFFilter* f, LFSearchResult* res)
{
	assert(f);
	assert(f->Mode>=LFFilterModeDirectoryTree);
	assert(res);

	if (!LoadTable(IDMaster))
	{
		res->m_LastError = LFIndexTableLoadError;
		return;
	}

	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		int pass = PassesFilterCore(PtrM, f);
		if (pass==-1)
			continue;

		// Master
		LFItemDescriptor* i = LFAllocItemDescriptor();
		i->Type = LFTypeFile;
		strcpy_s(i->StoreID, LFKeySize, StoreID);
		Tables[IDMaster]->WriteToItemDescriptor(i, PtrM);

		if (!f->Options.IgnoreSlaves)
		{
			// Slave
			if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
				if (LoadTable(PtrM->SlaveID))
				{
					void* PtrS;

					if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
						Tables[PtrM->SlaveID]->WriteToItemDescriptor(i, PtrS);
				}
				else
				{
					res->m_LastError = LFIndexTableLoadError;
				}

			if (pass!=1)
				pass = PassesFilterSlaves(i, f) ? 1 : -1;
		}

		if (pass!=-1)
			if (res->AddItemDescriptor(i))
				continue;

		// Nicht gesucht
		LFFreeItemDescriptor(i);
	}
}

unsigned int CIndex::RetrieveStats(unsigned int* cnt, __int64* size)
{
	if (!LoadTable(IDMaster))
		return LFIndexTableLoadError;

	int ID = 0;
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(ID, (void*&)PtrM))
	{
		#define Count(Domain) { if (cnt) cnt[Domain]++; if (size) size[Domain] += PtrM->FileSize; }

		if (PtrM->Flags & LFFlagTrash)
		{
			Count(LFDomainTrash);
		}
		else
		{
			Count(LFDomainAllFiles);
			if ((PtrM->DomainID>=LFDomainAudio) && (PtrM->DomainID<=LFDomainVideos))
				Count(LFDomainAllMediaFiles);
			if (PtrM->DomainID==LFDomainPhotos)
				Count(LFDomainPictures);
			if (PtrM->Rating)
				Count(LFDomainFavorites);

			Count(((PtrM->DomainID>=LFFirstPhysicalDomain) && (PtrM->DomainID<LFDomainCount)) ? PtrM->DomainID : LFDomainUnknown);
		}
	}

	return LFOk;
}
