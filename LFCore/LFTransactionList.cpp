
#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include "Mutex.h"
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
	m_Modified = m_Resolved = FALSE;
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

void LFTransactionList::SetError(CHAR* StoreID, UINT Result, LFProgress* pProgress)
{
	for (UINT a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].StoreID, StoreID)==0)
			{
				m_Items[a].LastError = m_LastError = Result;
				m_Items[a].Processed = TRUE;

				if (pProgress)
				{
					pProgress->Object[0] = L'\0';
					pProgress->MinorCurrent++;
				}
			}

	if (pProgress)
	{
		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		if (UpdateProgress(pProgress))
			m_LastError = LFCancel;
	}
}

void LFTransactionList::SetError(UINT Index, UINT Result, LFProgress* pProgress)
{
	m_Items[Index].LastError = m_LastError = Result;
	m_Items[Index].Processed = TRUE;

	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, m_Items[Index].pItemDescriptor ? m_Items[Index].pItemDescriptor->CoreAttributes.FileName : L"");
		pProgress->MinorCurrent++;

		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		if (UpdateProgress(pProgress))
			m_LastError = LFCancel;
	}
}

HGLOBAL LFTransactionList::CreateDropFiles()
{
	if (!m_Resolved)
		return NULL;

	SIZE_T cChars = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
			cChars += wcslen(&m_Items[a].Path[4])+1;

	SIZE_T szBuffer = sizeof(DROPFILES)+sizeof(WCHAR)*(cChars+1);
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hGlobal)
		return NULL;

	DROPFILES* pDropFiles = (DROPFILES*)GlobalLock(hGlobal);
	if (!pDropFiles)
	{
		GlobalFree(hGlobal);
		return NULL;
	}

	pDropFiles->pFiles = sizeof(DROPFILES);
	pDropFiles->fNC = TRUE;
	pDropFiles->pt.x = pDropFiles->pt.y = 0;
	pDropFiles->fWide = TRUE;

	WCHAR* Ptr = (WCHAR*)(((BYTE*)pDropFiles)+sizeof(DROPFILES));
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
		{
#pragma warning(push)
#pragma warning(disable: 4996)
			wcscpy(Ptr, &m_Items[a].Path[4]);
#pragma warning(pop)

			Ptr += wcslen(&m_Items[a].Path[4])+1;
		}

	*Ptr = L'\0';

	GlobalUnlock(hGlobal);

	return hGlobal;
}

HGLOBAL LFTransactionList::CreateLiquidFiles()
{
	if (!m_Resolved)
		return NULL;

	UINT cFiles = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
			cFiles++;

	SIZE_T szBuffer = sizeof(LIQUIDFILES)+sizeof(CHAR)*cFiles*LFKeySize*2;
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hGlobal)
		return NULL;

	LIQUIDFILES* pLiquidFiles = (LIQUIDFILES*)GlobalLock(hGlobal);
	if (!pLiquidFiles)
	{
		GlobalFree(hGlobal);
		return NULL;
	}

	pLiquidFiles->pFiles = sizeof(LIQUIDFILES);
	pLiquidFiles->cFiles = cFiles;

	CHAR* Ptr = (CHAR*)(((BYTE*)pLiquidFiles)+sizeof(LIQUIDFILES));
	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
		{
			strcpy_s(Ptr, LFKeySize, m_Items[a].pItemDescriptor->StoreID);
			strcpy_s(Ptr+LFKeySize, LFKeySize, m_Items[a].pItemDescriptor->CoreAttributes.FileID);

			Ptr += 2*LFKeySize;
		}

	GlobalUnlock(hGlobal);

	return hGlobal;
}
