
#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include <assert.h>


LFTransactionList::LFTransactionList()
	: DynArray()
{
	m_Changes = m_Resolved = false;
}

LFTransactionList::~LFTransactionList()
{
	if (m_Items)
		for (unsigned int a=0; a<m_ItemCount; a++)
			LFFreeItemDescriptor(m_Items[a].Item);
}

bool LFTransactionList::AddItemDescriptor(LFItemDescriptor* i, unsigned int UserData)
{
	assert(i);

	LFTL_Item item = { i, LFOk, UserData, false };

	if (!DynArray::AddItem(item))
		return false;

	i->RefCount++;
	return true;
}

void LFTransactionList::Reset()
{
	for (unsigned int a=0; a<m_ItemCount; a++)
	{
		m_Items[a].LastError = LFOk;
		m_Items[a].Processed = false;
	}

	m_LastError = LFOk;
}

void LFTransactionList::SetError(char* key, unsigned int res, LFProgress* pProgress)
{
	bool found = false;

	for (unsigned int a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].Item->StoreID, key)==0)
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

void LFTransactionList::SetError(unsigned int idx, unsigned int res, LFProgress* pProgress)
{
	m_Items[idx].LastError = m_LastError = res;
	m_Items[idx].Processed = true;

	if (pProgress)
	{
		if (res>LFCancel)
			pProgress->ProgressState = LFProgressError;

		wcscpy_s(pProgress->Object, 256, m_Items[idx].Item->CoreAttributes.FileName);
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}

LPITEMIDLIST LFTransactionList::DetachPIDL(unsigned int idx)
{
	assert(idx<m_ItemCount);

	LPITEMIDLIST pidl = m_Items[idx].pidlFQ;
	m_Items[idx].pidlFQ = NULL;

	return pidl;
}

HGLOBAL LFTransactionList::CreateDropFiles()
{
	if (!m_Resolved)
		return NULL;

	unsigned int cChars = 0;
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
			cChars += (unsigned int)wcslen(&m_Items[a].Path[4])+1;

	unsigned int szBuffer = sizeof(DROPFILES)+sizeof(wchar_t)*(cChars+1);
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

	wchar_t* ptr = (wchar_t*)(((unsigned char*)pDrop)+sizeof(DROPFILES));
	for (unsigned int a=0; a<m_ItemCount; a++)
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

	unsigned int cFiles = 0;
	for (unsigned int a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
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
