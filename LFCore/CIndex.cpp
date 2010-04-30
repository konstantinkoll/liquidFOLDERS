
#pragma once
#include "stdafx.h"
#include "CIndex.h"
#include "LFItemDescriptor.h"
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

inline void CIndex::LoadTable(unsigned int ID)
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
		}
}

void CIndex::AddItem(LFItemDescriptor* i)
{
	assert(i);

	// Master
	LoadTable(IDMaster);
	Tables[IDMaster]->Add(i);

	// Slave
	if (i->CoreAttributes.SlaveID)
	{
		LoadTable(i->CoreAttributes.SlaveID);
		Tables[i->CoreAttributes.SlaveID]->Add(i);
	}
}

void CIndex::Update(LFItemDescriptor* i)
{
	assert(i);

	// Master
	LoadTable(IDMaster);
	Tables[IDMaster]->Update(i);

	// Slave
	if (i->CoreAttributes.SlaveID)
	{
		LoadTable(i->CoreAttributes.SlaveID);
		Tables[i->CoreAttributes.SlaveID]->Update(i);
	}
}

void CIndex::Update(LFTransactionList* /*li*/)
{
	// TODO
}

void CIndex::Remove(LFItemDescriptor* i)
{
	assert(i);

	// Master
	LoadTable(IDMaster);
	Tables[IDMaster]->Invalidate(i);

	// Slave
	if (i->CoreAttributes.SlaveID)
	{
		LoadTable(i->CoreAttributes.SlaveID);
		Tables[i->CoreAttributes.SlaveID]->Invalidate(i);
	}
}

void CIndex::Remove(LFTransactionList* /*li*/)
{
	// TODO
}

void CIndex::RemoveTrash()
{
	LoadTable(IDMaster);

	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* Ptr;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)Ptr))
	{
		if (Ptr->Flags & LFFlagTrash)
		{
			// Slave
			if (Ptr->SlaveID)
			{
				LoadTable(Ptr->SlaveID);
				Tables[Ptr->SlaveID]->Invalidate(Ptr->FileID, IDs[Ptr->SlaveID]);
			}

			// Master
			Tables[IDMaster]->Invalidate(Ptr);
		}
	}
}

void CIndex::Compact(bool ForceAllTables)
{
	for (unsigned int a=0; a<IdxTableCount; a++)
	{
		if (ForceAllTables)
			LoadTable(a);

		if (Tables[a])
			Tables[a]->Compact();
	}
}

void CIndex::Retrieve(LFFilter* f, LFSearchResult* res)
{
	assert(f);
	assert(f->Mode>=LFFilterModeSearchInStore);
	assert(res);

	LoadTable(IDMaster);

	int IDs[IdxTableCount];
	ZeroMemory(IDs, sizeof(IDs));
	LFCoreAttributes* PtrM;

	while (Tables[IDMaster]->FindNext(IDs[IDMaster], (void*&)PtrM))
	{
		// Master
		LFItemDescriptor* i = LFAllocItemDescriptor();
		Tables[IDMaster]->WriteToItemDescriptor(i, PtrM);

		// Slave
		if (PtrM->SlaveID)
		{
			LoadTable(PtrM->SlaveID);
			void* PtrS;

			if (Tables[PtrM->SlaveID]->FindKey(PtrM->FileID, IDs[PtrM->SlaveID], PtrS))
				Tables[PtrM->SlaveID]->WriteToItemDescriptor(i, PtrS);
		}

		// Filter
		// TODO
		if (false)
			AttributesToString(i);
		if (true)
		{
			if (true)
				AttributesToString(i);
			res->AddItemDescriptor(i);
		}
	}
}
