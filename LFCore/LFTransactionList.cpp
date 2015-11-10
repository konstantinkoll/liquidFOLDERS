
#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include "Stores.h"
#include <assert.h>


extern LFMessageIDs LFMessages;


LFCORE_API LFTransactionList* LFAllocTransactionList(HLIQUID hLiquid)
{
	LFTransactionList* pTransactionList = new LFTransactionList();

	if (hLiquid)
	{
		LIQUIDFILES* pLiquidFiles = (LIQUIDFILES*)GlobalLock(hLiquid);

		if (pLiquidFiles)
		{
			UINT cFiles = pLiquidFiles->cFiles;
			CHAR* Ptr = (CHAR*)pLiquidFiles+sizeof(LIQUIDFILES);

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

LFCORE_API BOOL LFAddTransactionItemEx(LFTransactionList* pTransactionList, const CHAR* pStoreID, const CHAR* pFileID, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pTransactionList);

	return pTransactionList->AddItem(pStoreID, pFileID, pItemDescriptor, UserData);
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

LFCORE_API void LFDoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress, UINT_PTR Parameter, LFVariantData* pVariantData1, LFVariantData* pVariantData2, LFVariantData* pVariantData3)
{
	assert(pTransactionList);

	pTransactionList->DoTransaction(TransactionType, pProgress, Parameter, pVariantData1, pVariantData2, pVariantData3);
}


// LFTransactionList
//

LFTransactionList::LFTransactionList()
	: LFDynArray()
{
	m_LastError = LFOk;
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

BOOL LFTransactionList::AddItem(const CHAR* pStoreID, const CHAR* pFileID, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pStoreID);
	assert(pFileID);

	LFTransactionListItem Item;

	strcpy_s(Item.StoreID, LFKeySize, pStoreID);
	strcpy_s(Item.FileID, LFKeySize, pFileID);
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

void LFTransactionList::SetError(const CHAR* pStoreID, UINT Result, LFProgress* pProgress)
{
	if (Result==LFOk)
		m_Modified = TRUE;

	for (UINT a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed)
			if (strcmp(m_Items[a].StoreID, pStoreID)==0)
			{
				if (Result!=LFOk)
					m_LastError = Result;

				m_Items[a].LastError = Result;
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
	if (Result==LFOk)
	{
		m_Modified = TRUE;
	}
	else
	{
		m_LastError = Result;
	}

	m_Items[Index].LastError = Result;
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
	UINT cFiles = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
			cFiles++;

	SIZE_T szBuffer = sizeof(LIQUIDFILES)+cFiles*LFKeySize*2;
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hGlobal)
		return NULL;

	LIQUIDFILES* pLiquidFiles = (LIQUIDFILES*)GlobalLock(hGlobal);
	if (!pLiquidFiles)
	{
		GlobalFree(hGlobal);
		return NULL;
	}

	pLiquidFiles->cFiles = cFiles;
	pLiquidFiles->pFiles = sizeof(LIQUIDFILES);

	CHAR* Ptr = (CHAR*)pLiquidFiles+sizeof(LIQUIDFILES);
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
		{
			strcpy_s(Ptr, LFKeySize, m_Items[a].StoreID);
			strcpy_s(Ptr+LFKeySize, LFKeySize, m_Items[a].FileID);

			Ptr += 2*LFKeySize;
		}

	GlobalUnlock(hGlobal);

	return hGlobal;
}

BOOL LFTransactionList::SetStoreAttributes(LFVariantData* pVariantData, WCHAR** ppStoreName, WCHAR** ppStoreComments)
{
	assert(ppStoreName);
	assert(ppStoreComments);

	if (!pVariantData)
		return TRUE;


	switch (pVariantData->Attr)
	{
	case LFAttrFileName:
		assert(pVariantData->Type==LFTypeUnicodeString);

		if (pVariantData->IsNull)
			return FALSE;

		*ppStoreName = pVariantData->UnicodeString;

		return TRUE;

	case LFAttrComments:
		assert(pVariantData->Type==LFTypeUnicodeString);
		*ppStoreComments = pVariantData->UnicodeString;

		return TRUE;

	default:
		return FALSE;
	}
}

void LFTransactionList::DoTransaction(UINT TransactionType, LFProgress* pProgress, UINT_PTR Parameter, LFVariantData* pVariantData1, LFVariantData* pVariantData2, LFVariantData* pVariantData3)
{
	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = m_ItemCount;
		pProgress->MinorCurrent = 0;
	}

	// Process items
	UINT Result;
	CStore* pStore;

	for (UINT a=0; a<m_ItemCount; a++)
		if ((m_Items[a].LastError==LFOk) && (!m_Items[a].Processed))
		{
			switch (m_Items[a].pItemDescriptor ? m_Items[a].pItemDescriptor->Type & LFTypeMask : LFTypeFile)
			{
			case LFTypeFile:
				if ((Result=OpenStore(m_Items[a].StoreID, TransactionType>LFTransactionTypeLastReadonly, &pStore))==LFOk)
				{
					pStore->DoTransaction(this, TransactionType, pProgress, Parameter, pVariantData1, pVariantData2, pVariantData3);
					delete pStore;
				}
				else
				{
					SetError(m_Items[a].StoreID, Result, pProgress);
				}

				break;

			case LFTypeStore:
				if (TransactionType==LFTransactionTypeUpdate)
				{
					WCHAR* pStoreName = NULL;
					WCHAR* pStoreComments = NULL;

					if (!SetStoreAttributes(pVariantData1, &pStoreName, &pStoreComments))
					{
						SetError(a, LFIllegalAttribute, pProgress);
						break;
					}

					if (!SetStoreAttributes(pVariantData2, &pStoreName, &pStoreComments))
					{
						SetError(a, LFIllegalAttribute, pProgress);
						break;
					}

					if (!SetStoreAttributes(pVariantData3, &pStoreName, &pStoreComments))
					{
						SetError(a, LFIllegalAttribute, pProgress);
						break;
					}

					SetError(a, LFSetStoreAttributes(m_Items[a].StoreID, pStoreName, pStoreComments), pProgress);

					break;
				}

			default:
				SetError(a, LFIllegalItemType, pProgress);
			}

			// Progress
			if (pProgress)
				if (pProgress->UserAbort)
					break;
		}

	// Get PIDLs
	if (TransactionType==LFTransactionTypeResolveLocations)
	{
		if ((BOOL)Parameter)
			for (UINT a=0; a<m_ItemCount; a++)
				if ((m_Items[a].Processed) && (m_Items[a].LastError==LFOk))
					if (FAILED(SHParseDisplayName(&m_Items[a].Path[4], NULL, &m_Items[a].pidlFQ, 0, NULL)))
						m_Items[a].LastError = m_LastError = LFIllegalPhysicalPath;

		m_Resolved = TRUE;
	}

	// Finish transaction
	switch (TransactionType)
	{
	case LFTransactionTypeSendTo:
	case LFTransactionTypeArchive:
	case LFTransactionTypePutInTrash:
	case LFTransactionTypeRestore:
	case LFTransactionTypeUpdate:
	case LFTransactionTypeDelete:
		SendLFNotifyMessage(LFMessages.StatisticsChanged);

		break;
	}
}
