
#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include <assert.h>


LFTransactionList::LFTransactionList()
	: LFDynArray()
{
	m_Changes = m_Resolved = FALSE;
}

LFTransactionList::~LFTransactionList()
{
	if (m_Items)
		for (UINT a=0; a<m_ItemCount; a++)
			LFFreeItemDescriptor(m_Items[a].Item);
}

BOOL LFTransactionList::AddItemDescriptor(LFItemDescriptor* i, UINT UserData)
{
	assert(i);

	LFTL_Item item = { "", "", i, UserData, LFOk, FALSE };

	if (!LFDynArray::AddItem(item))
		return FALSE;

	i->RefCount++;
	return TRUE;
}

void LFTransactionList::Reset()
{
	for (UINT a=0; a<m_ItemCount; a++)
	{
		m_Items[a].LastError = LFOk;
		m_Items[a].Processed = FALSE;
	}

	m_LastError = LFOk;
}

void LFTransactionList::SetError(CHAR* key, UINT Result, LFProgress* pProgress)
{
	BOOL found = FALSE;

	for (UINT a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].Item->StoreID, key)==0)
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

void LFTransactionList::SetError(UINT idx, UINT Result, LFProgress* pProgress)
{
	m_Items[idx].LastError = m_LastError = Result;
	m_Items[idx].Processed = TRUE;

	if (pProgress)
	{
		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		wcscpy_s(pProgress->Object, 256, m_Items[idx].Item->CoreAttributes.FileName);
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}

HGLOBAL LFTransactionList::CreateDropFiles()
{
	if (!m_Resolved)
		return NULL;

	UINT cChars = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
			cChars += (UINT)wcslen(&m_Items[a].Path[4])+1;

	UINT szBuffer = sizeof(DROPFILES)+sizeof(WCHAR)*(cChars+1);
	HGLOBAL hG = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hG)
		return NULL;

	DROPFILES* pDrop = (DROPFILES*)GlobalLock(hG);
	if (!pDrop)
	{
		GlobalFree(hG);
		return NULL;
	}

	pDrop->pFiles = sizeof(DROPFILES);
	pDrop->fNC = TRUE;
	pDrop->pt.x = pDrop->pt.y = 0;
	pDrop->fWide = TRUE;

	WCHAR* ptr = (WCHAR*)(((BYTE*)pDrop)+sizeof(DROPFILES));
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
		{
#pragma warning(push)
#pragma warning(disable: 4996)
			wcscpy(ptr, &m_Items[a].Path[4]);
#pragma warning(pop)
			ptr += wcslen(&m_Items[a].Path[4])+1;
		}

	*ptr = L'\0';

	GlobalUnlock(hG);
	return hG;
}

HGLOBAL LFTransactionList::CreateLiquidFiles()
{
	if (!m_Resolved)
		return NULL;

	UINT cFiles = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
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
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
		{
			strcpy_s(ptr, LFKeySize, m_Items[a].Item->StoreID);
			ptr += LFKeySize;
			strcpy_s(ptr, LFKeySize, m_Items[a].Item->CoreAttributes.FileID);
			ptr += LFKeySize;
		}

	GlobalUnlock(hG);
	return hG;
}
