
#pragma once
#include "stdafx.h"
#include "CIndex.h"
#include "LFItemDescriptor.h"
#include "Query.h"
#include "Stores.h"
#include <assert.h>


CIndex::CIndex(char* _Path, char* _StoreID)
{
	strcpy_s(Path, MAX_PATH, _Path);
	strcpy_s(StoreID, LFKeySize, _StoreID);
	ZeroMemory(Tables, sizeof(Tables));
}

CIndex::~CIndex()
{
	for (unsigned int a=0; a<IdxTableCount; a++)
		if (Tables[a])
			delete Tables[a];
}

inline bool CIndex::LoadTable(unsigned int ID, unsigned int* res)
{
	assert(ID<IdxTableCount);

	if (!Tables[ID])
		switch (ID)
		{
		case IDMaster:
			Tables[ID] = new CIdxTableMaster(Path, "Master.idx");
			break;
		case IDSlaveDocuments:
			Tables[ID] = new CIdxTableDocuments(Path, "Documents.idx");
			break;
		case IDSlaveMails:
			Tables[ID] = new CIdxTableMails(Path, "Mails.idx");
			break;
		case IDSlaveAudio:
			Tables[ID] = new CIdxTableAudio(Path, "Audio.idx");
			break;
		case IDSlavePictures:
			Tables[ID] = new CIdxTablePictures(Path, "Pictures.idx");
			break;
		case IDSlaveVideos:
			Tables[ID] = new CIdxTableVideos(Path, "Videos.idx");
			break;
		default:
			assert(false);

			if (res)
				*res = HeapError;
			return false;
		}

	if (res)
		*res = Tables[ID]->OpenStatus;

	return (Tables[ID]->OpenStatus!=HeapError) && (Tables[ID]->OpenStatus!=HeapCannotCreate);
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
	bool Reindex = false;
	bool Repaired = false;
	unsigned int tres[IdxTableCount];
	unsigned int MaxSize = 0;

	// Tabellen prüfen
	for (unsigned int a=0; a<IdxTableCount; a++)
	{
		LoadTable(a, &tres[a]);
		switch (tres[a])
		{
		case HeapCannotCreate:
		case HeapError:
			return IndexError;
		case HeapCreated:
			if (a==IDMaster)
				return IndexReindexRequired;
			Reindex = true;
			Repaired = true;
			break;
		case HeapMaintenanceRequired:
			if (!Tables[a]->Compact())
				return IndexError;
			tres[a] = Tables[a]->OpenStatus;
			Repaired = true;
			break;
		}

		MaxSize = max(MaxSize, Tables[a]->GetRequiredElementSize());
	}

	// Index-Durchlauf
	if (scheduled || Reindex)
	{
		if (!DirFreeSpace(Path, MaxSize*Tables[IDMaster]->GetItemCount()))
			return IndexNotEnoughFreeDiscSpace;

		// TODO: registrierte Formate anpassen, ggf. neue Slaves erstellen
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

void CIndex::Update(LFItemDescriptor* i)
{
	assert(i);

	i->CoreAttributes.Flags &= !LFFlagNew;

	// Master
	LoadTable(IDMaster);
	Tables[IDMaster]->Update(i);

	// Slave
	if ((i->CoreAttributes.SlaveID) && (i->CoreAttributes.SlaveID<IdxTableCount))
	{
		LoadTable(i->CoreAttributes.SlaveID);
		Tables[i->CoreAttributes.SlaveID]->Update(i);
	}
}

void CIndex::Update(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	assert(tl);

	if (!LoadTable(IDMaster))
	{
		tl->m_LastError = LFIndexRepairError;
		return;
	}

	// Items aktualisieren
	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		for (unsigned int a=0; a<tl->m_Count; a++)
		{
			LFItemDescriptor* i = tl->m_Entries[a].Item;
			if ((i->Type & LFTypeMask)==LFTypeFile)
				if ((strcmp(i->StoreID, StoreID)==0) && (strcmp(i->CoreAttributes.FileID, PtrM->FileID)==0))
				{
					if (tl->m_Entries[a].LastError==LFOk)
					{
						// Attribute setzen
						i->CoreAttributes.Flags &= !LFFlagNew;
						tl->m_Changes = true;

						LFSetAttributeVariantData(i, value1);
						if (value2)
							LFSetAttributeVariantData(i, value2);
						if (value3)
							LFSetAttributeVariantData(i, value3);

						// Master
						Tables[IDMaster]->Update(i, PtrM);

						// Slave
						if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
							if (LoadTable(PtrM->SlaveID))
							{
								void* PtrS;

								if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
									Tables[PtrM->SlaveID]->Update(i, PtrS);
							}
							else
							{
								tl->m_Entries[a].LastError = tl->m_LastError = LFIndexError;
							}

						tl->m_Entries[a].Processed = true;
					}
				}
		}
	}

	// Ungültige Items finden
	for (unsigned int a=0; a<tl->m_Count; a++)
	{
		LFItemDescriptor* i = tl->m_Entries[a].Item;
		if ((i->Type & LFTypeMask)==LFTypeFile)
			if ((strcmp(i->StoreID, StoreID)==0) && (!tl->m_Entries[a].Processed))
			{
				tl->m_Entries[a].LastError = tl->m_LastError = LFIllegalKey;
				tl->m_Entries[a].Processed = true;
			}
	}
}

void CIndex::Delete(LFTransactionList* tl, char* DatPath)
{
	assert(tl);

	if (!LoadTable(IDMaster))
	{
		tl->m_LastError = LFIndexRepairError;
		return;
	}

	// Items löschen
	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		for (unsigned int a=0; a<tl->m_Count; a++)
		{
			LFItemDescriptor* i = tl->m_Entries[a].Item;
			if ((i->Type & LFTypeMask)==LFTypeFile)
				if ((strcmp(i->StoreID, StoreID)==0) && (strcmp(i->CoreAttributes.FileID, PtrM->FileID)==0))
				{
					// Files with "link" flag do not posses a file body
					if ((!tl->m_Entries[a].Processed) && ((i->CoreAttributes.Flags & LFFlagLink)==0))
					{
						char Path[MAX_PATH];
						GetFileLocation(DatPath, PtrM->FileID, PtrM->FileFormat, Path, MAX_PATH);

						if (DeleteFileA(Path))
						{
							tl->m_Changes = true;
						}
						else
						{
							tl->m_Entries[a].LastError = tl->m_LastError = LFCannotDeleteFile;
						}
					}

					if (tl->m_Entries[a].LastError==LFOk)
					{
						// Slave
						if ((PtrM->SlaveID) && (PtrM->SlaveID<IdxTableCount))
							if (LoadTable(PtrM->SlaveID))
							{
								Tables[PtrM->SlaveID]->Invalidate(PtrM->FileID, IDs[PtrM->SlaveID]);
							}
							else
							{
								tl->m_Entries[a].LastError = tl->m_LastError = LFIndexError;
							}

						// Master
						Tables[IDMaster]->Invalidate(PtrM);
					}

					tl->m_Entries[a].Processed = true;
				}
		}
	}

	// Ungültige Items finden
	for (unsigned int a=0; a<tl->m_Count; a++)
	{
		LFItemDescriptor* i = tl->m_Entries[a].Item;
		if ((i->Type & LFTypeMask)==LFTypeFile)
			if ((strcmp(i->StoreID, StoreID)==0) && (!tl->m_Entries[a].Processed))
			{
				tl->m_Entries[a].LastError = tl->m_LastError = LFIllegalKey;
				tl->m_Entries[a].Processed = true;
			}
	}
}

void CIndex::Retrieve(LFFilter* f, LFSearchResult* res)
{
	assert(f);
	assert(f->Mode>=LFFilterModeDirectoryTree);
	assert(res);

	if (!LoadTable(IDMaster))
	{
		res->m_LastError = LFIndexRepairError;
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
		i->IconID = IDI_FILE_Generic;
		strcpy_s(i->StoreID, LFKeySize, StoreID);
		Tables[IDMaster]->WriteToItemDescriptor(i, PtrM);

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
				res->m_LastError = LFIndexError;
			}

		if (pass!=1)
			pass = PassesFilterSlaves(i, f) ? 1 : -1;

		if (pass==1)
			if (res->AddItemDescriptor(i))
				continue;

		// Nicht gesucht
		LFFreeItemDescriptor(i);
	}
}

void CIndex::RetrieveStats(unsigned int* cnt, __int64* size)
{
	if (!LoadTable(IDMaster))
		return;

	int ID = 0;
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(ID, (void*&)PtrM))
	{
		#define Count(Domain) { if (cnt) cnt[Domain]++; if (size) size[Domain]+=PtrM->FileSize; }

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
}
