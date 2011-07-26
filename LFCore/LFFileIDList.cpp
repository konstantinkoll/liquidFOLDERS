#include "stdafx.h"
#include "LFCore.h"
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

	LFFIL1_Item item;
	ZeroMemory(&item, sizeof(item));
	strcpy_s(item.StoreID, LFKeySize, StoreID);
	strcpy_s(item.FileID, LFKeySize, FileID);
	item.UserData = UserData;

	return DynArray::AddItem(item);
}

void LFFileIDList::Reset()
{
	for (unsigned int a=0; a<m_ItemCount; a++)
	{
		m_Items[a].LastError = LFOk;
		m_Items[a].Processed = false;
	}

	m_LastError = LFOk;
}

void LFFileIDList::SetError(char* key, unsigned int res)
{
	for (unsigned int a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].StoreID, key)==0)
			{
				m_Items[a].LastError = m_LastError = res;
				m_Items[a].Processed = true;
			}
}

HGLOBAL LFFileIDList::CreateLiquidFiles()
{
	unsigned int cFiles = 0;
	for (unsigned int a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
			cFiles++;

	unsigned int szBuffer = sizeof(LIQUIDFILES)+sizeof(char)*cFiles*LFKeySize*2;
	HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hG)
		return NULL;

	LIQUIDFILES* pFiles = (LIQUIDFILES*)GlobalLock(hG);
	if (!pFiles)
	{
		GlobalFree(hG);
		return NULL;
	}

	pFiles->pFiles = sizeof(LIQUIDFILES);
	pFiles->cFiles = cFiles;

	char* ptr = (char*)(((unsigned char*)pFiles)+sizeof(LIQUIDFILES));
	for (unsigned int a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
		{
			strcpy_s(ptr, LFKeySize, m_Items[a].StoreID);
			ptr += LFKeySize;
			strcpy_s(ptr, LFKeySize, m_Items[a].FileID);
			ptr += LFKeySize;
		}

	GlobalUnlock(hG);
	return hG;
}
