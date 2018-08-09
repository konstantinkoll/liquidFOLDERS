
#include "stdafx.h"
#include "LFCore.h"
#include "LFTransactionList.h"
#include "Stores.h"


extern LFMessageIDs LFMessages;


LFCORE_API LFTransactionList* LFAllocTransactionList(LFSearchResult* pSearchResult, BOOL All)
{
	LFTransactionList* pTransactionList = new LFTransactionList();

	if (pSearchResult)
		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
		{
			const UINT Type = (*pSearchResult)[a]->Type;

			if (All || (Type & LFTypeSelected))
			{
				assert(((Type & LFTypeMask)==LFTypeFile) || ((Type & LFTypeMask)==LFTypeStore));
				pTransactionList->AddItem((*pSearchResult)[a], a);
			}
		}

	return pTransactionList;
}

LFCORE_API LFTransactionList* LFAllocTransactionListEx(HLIQUIDFILES hLiquidFiles)
{
	assert(hLiquidFiles);

	LFTransactionList* pTransactionList = new LFTransactionList();

	LPCLIQUIDFILES lpcLiquidFiles = (LPCLIQUIDFILES)GlobalLock(hLiquidFiles);
	if (lpcLiquidFiles)
	{
		for (UINT a=0; a<lpcLiquidFiles->cFiles; a++)
			pTransactionList->AddItem(lpcLiquidFiles->FileItems[a]);

		GlobalUnlock(hLiquidFiles);
	}

	return pTransactionList;
}

LFCORE_API void LFFreeTransactionList(LFTransactionList* pTransactionList)
{
	delete pTransactionList;
}

LFCORE_API BOOL LFAddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pTransactionList);

	return pTransactionList->AddItem(pItemDescriptor, UserData);
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

LFCORE_API UINT LFDoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress, UINT_PTR Parameter, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2, const LFVariantData* pVariantData3)
{
	assert(pTransactionList);

	return pTransactionList->DoTransaction(TransactionType, pProgress, Parameter, pVariantData1, pVariantData2, pVariantData3);
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

BOOL LFTransactionList::AddItem(const ABSOLUTESTOREID& StoreID, const FILEID& FileID, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	LFTransactionListItem Item;

	Item.StoreID = StoreID;
	Item.FileID = FileID;
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

void LFTransactionList::SetError(const ABSOLUTESTOREID& StoreID, UINT Result, LFProgress* pProgress)
{
	if (Result==LFOk)
		m_Modified = TRUE;

	for (UINT a=0; a<m_ItemCount; a++)
		if (!m_Items[a].Processed && (m_Items[a].StoreID==StoreID))
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

	assert(Index<m_ItemCount);
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

	// Buffer size
	SIZE_T cChars = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].Processed && (m_Items[a].LastError==LFOk))
			cChars += wcslen(&m_Items[a].Path[4])+1;

	// Alloc memory
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

	// Set up data structure
	pDropFiles->pFiles = sizeof(DROPFILES);
	pDropFiles->pt.x = pDropFiles->pt.y = 0;
	pDropFiles->fWide = pDropFiles->fNC = TRUE;

	// Paths
	LPWSTR pPath = (LPWSTR)(((LPBYTE)pDropFiles)+sizeof(DROPFILES));
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].Processed && (m_Items[a].LastError==LFOk))
		{
#pragma warning(push)
#pragma warning(disable: 4996)
			wcscpy(pPath, &m_Items[a].Path[4]);
#pragma warning(pop)

			pPath += wcslen(&m_Items[a].Path[4])+1;
		}

	*pPath = L'\0';

	GlobalUnlock(hGlobal);

	return hGlobal;
}

HGLOBAL LFTransactionList::CreateLiquidFiles()
{
	// File count
	UINT cFiles = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
			cFiles++;

	// Alloc memory
	const SIZE_T szBuffer = sizeof(LIQUIDFILES)+(cFiles-1)*sizeof(LIQUIDFILEITEM);
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, szBuffer);
	if (!hGlobal)
		return NULL;

	LPLIQUIDFILES lpLiquidFiles = (LPLIQUIDFILES)GlobalLock(hGlobal);
	if (!lpLiquidFiles)
	{
		GlobalFree(hGlobal);

		return NULL;
	}

	// Set up data structure
	lpLiquidFiles->cFiles = cFiles;

	// IDs
	UINT Index = 0;
	for (UINT a=0; a<m_ItemCount; a++)
		if (m_Items[a].LastError==LFOk)
			lpLiquidFiles->FileItems[Index++] = m_Items[a].FileItem;

	assert(Index==cFiles);
	GlobalUnlock(hGlobal);

	return hGlobal;
}

BOOL LFTransactionList::SetStoreAttributes(const LFVariantData* pVData, LPCWSTR* ppStoreName, LPCWSTR* ppStoreComments)
{
	assert(ppStoreName);
	assert(ppStoreComments);

	if (!pVData)
		return TRUE;

	switch (pVData->Attr)
	{
	case LFAttrComments:
		assert(pVData->Type==LFTypeUnicodeString);

		*ppStoreComments = pVData->UnicodeString;

		return TRUE;

	case LFAttrFileName:
		assert(pVData->Type==LFTypeUnicodeString);

		if (!pVData->IsNull)
		{
			*ppStoreName = pVData->UnicodeString;

			return TRUE;
		}
	}

	return FALSE;
}

UINT LFTransactionList::DoTransaction(UINT TransactionType, LFProgress* pProgress, UINT_PTR Parameter, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2, const LFVariantData* pVariantData3)
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
		if ((m_Items[a].LastError==LFOk) && !m_Items[a].Processed)
		{
			switch (m_Items[a].pItemDescriptor ? m_Items[a].pItemDescriptor->Type & LFTypeMask : LFTypeFile)
			{
			case LFTypeFile:
				if ((Result=OpenStore(m_Items[a].StoreID, pStore, TransactionType>LFTransactionLastReadonly))==LFOk)
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
				if (TransactionType==LFTransactionUpdate)
				{
					LPCWSTR pStoreName = NULL;
					LPCWSTR pStoreComments = NULL;

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
	if (TransactionType==LFTransactionResolveLocations)
	{
		if ((BOOL)Parameter)
			for (UINT a=0; a<m_ItemCount; a++)
				if (m_Items[a].Processed && (m_Items[a].LastError==LFOk))
					if (FAILED(SHParseDisplayName(&m_Items[a].Path[4], NULL, &m_Items[a].pidlFQ, 0, NULL)))
						m_Items[a].LastError = m_LastError = LFIllegalPhysicalPath;

		m_Resolved = TRUE;
	}

	// Finish transaction
	switch (TransactionType)
	{
	case LFTransactionSendTo:
	case LFTransactionArchive:
	case LFTransactionPutInTrash:
	case LFTransactionRecover:
	case LFTransactionUpdate:
	case LFTransactionUpdateTask:
	case LFTransactionUpdateUserContext:
	case LFTransactionDelete:
		SendLFNotifyMessage(LFMessages.StatisticsChanged);

		break;
	}

	return m_LastError;
}
