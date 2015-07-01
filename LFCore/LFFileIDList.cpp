#include "stdafx.h"
#include "LFCore.h"
#include "LFFileIDList.h"
#include <assert.h>


LFFileIDList::LFFileIDList()
	: LFDynArray()
{
	m_Changes = FALSE;
}

BOOL LFFileIDList::AddFileID(CHAR* StoreID, CHAR* FileID, void* UserData)
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
	for (UINT a=0; a<m_ItemCount; a++)
	{
		m_Items[a].LastError = LFOk;
		m_Items[a].Processed = FALSE;
	}

	m_LastError = LFOk;
}

void LFFileIDList::SetError(CHAR* key, UINT Result, LFProgress* pProgress)
{
	BOOL found = FALSE;

	for (UINT a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].StoreID, key)==0)
			{
				found = TRUE;

				m_Items[a].LastError = m_LastError = Result;
				m_Items[a].Processed = TRUE;
				if (pProgress)
					pProgress->MinorCurrent++;
			}

	if (pProgress)
	{
		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;
		if (found)
			pProgress->Object[0] = L'\0';

		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}

void LFFileIDList::SetError(UINT idx, UINT Result, LFProgress* pProgress)
{
	m_Items[idx].LastError = m_LastError = Result;
	m_Items[idx].Processed = TRUE;

	if (pProgress)
	{
		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		pProgress->Object[0] = L'\0';
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}

HGLOBAL LFFileIDList::CreateLiquidFiles()
{
	UINT cFiles = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
			cFiles++;

	UINT szBuffer = sizeof(LIQUIDFILES)+sizeof(CHAR)*cFiles*LFKeySize*2;
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

	CHAR* ptr = (CHAR*)(((BYTE*)pFiles)+sizeof(LIQUIDFILES));
	for (UINT a=0; a<m_ItemCount; a++)
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
