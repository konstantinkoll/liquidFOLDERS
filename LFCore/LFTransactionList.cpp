
#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include <assert.h>


LFCORE_API LFTransactionList* LFAllocTransactionList(HLIQUID hLiquid)
{
	LFTransactionList* pTransactionList = new LFTransactionList();

	if (hLiquid)
	{
		LIQUIDFILES* pLiquidFiles = (LIQUIDFILES*)GlobalLock(hLiquid);
		
		if (pLiquidFiles)
		{
			UINT cFiles = pLiquidFiles->cFiles;
			CHAR* Ptr = (CHAR*)(((BYTE*)pLiquidFiles)+sizeof(LIQUIDFILES));

			for (UINT a=0; a<cFiles; a++)
			{
				pTransactionList->AddItem(Ptr, Ptr+LFKeySize);

				Ptr += 2*LFKeySize;
			}
		}

		GlobalUnlock(hLiquid);
	}

	return pTransactionList;
}

LFCORE_API void LFFreeTransactionList(LFTransactionList* pTransactionList)
{
	assert(pTransactionList);

	delete pTransactionList;
}

LFCORE_API BOOL LFAddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pTransactionList);

	return pTransactionList->AddItem(pItemDescriptor, UserData);
}

LFCORE_API BOOL LFAddTransactionItemEx(LFTransactionList* pTransactionList, CHAR* StoreID, CHAR* FileID, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pTransactionList);

	return pTransactionList->AddItem(StoreID, FileID, pItemDescriptor, UserData);
}

LFCORE_API HGLOBAL LFCreateDropFiles(LFTransactionList* pTransactionList)
{
	assert(pTransactionList);

	return pTransactionList->CreateDropFiles();
}

LFCORE_API HGLOBAL LFCreateLiquidFiles(LFTransactionList* pTransactionList)
{
	assert(pTransactionList);

	return pTransactionList->CreateLiquidFiles();
}


// LFTransactionList
//

LFTransactionList::LFTransactionList()
	: LFDynArray()
{
	m_Changes = m_Resolved = FALSE;
}

LFTransactionList::~LFTransactionList()
{
	if (m_Items)
		for (UINT a=0; a<m_ItemCount; a++)
			if (m_Items[a].pItemDescriptor)
				LFFreeItemDescriptor(m_Items[a].pItemDescriptor);
}

BOOL LFTransactionList::AddItem(LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pItemDescriptor);

	LFTransactionListItem Item;

	strcpy_s(Item.StoreID, LFKeySize, pItemDescriptor->StoreID);
	strcpy_s(Item.FileID, LFKeySize, pItemDescriptor->CoreAttributes.FileID);
	Item.pItemDescriptor = pItemDescriptor;
	Item.UserData = UserData;

	Item.LastError = 0;
	Item.Processed = FALSE;

	Item.Path[0] = L'\0';
	Item.pidlFQ = NULL;
	
	if (!LFDynArray::AddItem(Item))
		return FALSE;

	pItemDescriptor->RefCount++;

	return TRUE;
}

BOOL LFTransactionList::AddItem(CHAR* StoreID, CHAR* FileID, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(StoreID);
	assert(FileID);

	LFTransactionListItem Item;

	strcpy_s(Item.StoreID, LFKeySize, StoreID);
	strcpy_s(Item.FileID, LFKeySize, FileID);
	Item.pItemDescriptor = pItemDescriptor;
	Item.UserData = UserData;

	Item.LastError = 0;
	Item.Processed = FALSE;

	Item.Path[0] = L'\0';
	Item.pidlFQ = NULL;
	
	if (!LFDynArray::AddItem(Item))
		return FALSE;

	if (pItemDescriptor)
		pItemDescriptor->RefCount++;

	return TRUE;
}

void LFTransactionList::SetError(CHAR* key, UINT Result, LFProgress* pProgress)
{
	BOOL found = FALSE;

	for (UINT a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].pItemDescriptor->StoreID, key)==0)
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

		wcscpy_s(pProgress->Object, 256, m_Items[idx].pItemDescriptor->CoreAttributes.FileName);
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
			strcpy_s(ptr, LFKeySize, m_Items[a].pItemDescriptor->StoreID);
			ptr += LFKeySize;
			strcpy_s(ptr, LFKeySize, m_Items[a].pItemDescriptor->CoreAttributes.FileID);
			ptr += LFKeySize;
		}

	GlobalUnlock(hG);
	return hG;
}
