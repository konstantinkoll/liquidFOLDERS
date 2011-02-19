#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "LFFileIDList.h"
#include <assert.h>


LFFileIDList::LFFileIDList()
	: DynArray()
{
	m_Changes = false;
}

bool LFFileIDList::AddFileID(char* StoreID, char* FileID, void* UserData)
{
	assert(StoreID);
	assert(FileID);

	LFFIL_Item item;
	ZeroMemory(&item, sizeof(item));
	strcpy_s(item.StoreID, LFKeySize, StoreID);
	strcpy_s(item.FileID, LFKeySize, FileID);
	item.UserData = UserData;

	return DynArray::AddItem(item);
}
