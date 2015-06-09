#include "stdafx.h"
#include "LFCore.h"
#include "LFFileIDList.h"
#include <assert.h>


LFFileIDList::LFFileIDList()
	: LFDynArray()
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

	return LFDynArray::AddItem(item);
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

void LFFileIDList::SetError(char* key, unsigned int res, LFProgress* pProgress)
{
	bool found = false;

	for (unsigned int a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].StoreID, key)==0)
			{
				found = true;

				m_Items[a].LastError = m_LastError = res;
				m_Items[a].Processed = true;
				if (pProgress)
					pProgress->MinorCurrent++;
			}

	if (pProgress)
	{
		if (res>LFCancel)
			pProgress->ProgressState = LFProgressError;
		if (found)
			pProgress->Object[0] = L'\0';

		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}

void LFFileIDList::SetError(unsigned int idx, unsigned int res, LFProgress* pProgress)
{
	m_Items[idx].LastError = m_LastError = res;
	m_Items[idx].Processed = true;

	if (pProgress)
	{
		if (res>LFCancel)
			pProgress->ProgressState = LFProgressError;

		pProgress->Object[0] = L'\0';
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
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
