
#include "stdafx.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "Stores.h"
#include "Transaction.h"
#include <assert.h>
#include <shlobj.h>


extern LFMessageIDs LFMessages;

void UpdateStore(LFTransactionList* tl, unsigned int idx, LFVariantData* value, bool& Updated)
{
	unsigned int result = LFIllegalAttribute;

	switch (value->Attr)
	{
	case LFAttrFileName:
		result = LFSetStoreAttributes(tl->m_Items[idx].Item->StoreID, value->UnicodeString, NULL, NULL, true);
		break;
	case LFAttrComments:
		result = LFSetStoreAttributes(tl->m_Items[idx].Item->StoreID, NULL, value->UnicodeString, NULL, true);
		break;
	default:
		result = LFIllegalAttribute;
	}

	if (result==LFOk)
	{
		LFSetAttributeVariantData(tl->m_Items[idx].Item, value);
		tl->m_Changes = true;
		Updated |= true;
	}
	else
	{
		tl->m_LastError = tl->m_Items[idx].LastError = result;
	}
	tl->m_Items[idx].Processed = true;
}

LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	assert(tl);

	bool StoresUpdated = false;

	// Reset
	tl->Reset();

	// Process
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if (tl->m_Items[a].LastError==LFOk)
			switch (tl->m_Items[a].Item->Type & LFTypeMask)
			{
			case LFTypeStore:
				UpdateStore(tl, a, value1, StoresUpdated);
				if (value2)
					UpdateStore(tl, a, value2, StoresUpdated);
				if (value3)
					UpdateStore(tl, a, value3, StoresUpdated);
			case LFTypeFile:
				if (!tl->m_Items[a].Processed)
				{
					CIndex* idx1;
					CIndex* idx2;
					HANDLE StoreLock = NULL;
					unsigned int res = OpenStore(tl->m_Items[a].Item->StoreID, true, idx1, idx2, NULL, &StoreLock);

					if (res==LFOk)
					{
						if (idx1)
						{
							idx1->Update(tl, value1, value2, value3);
							delete idx1;
						}
						if (idx2)
						{
							idx2->Update(tl, value1, value2, value3);
							delete idx2;
						}

						ReleaseMutexForStore(StoreLock);
					}
					else
					{
						tl->SetError(tl->m_Items[a].Item->StoreID, res);
					}
				}

				break;
			default:
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = true;
			}

	// Update messages
	if (StoresUpdated)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores, hWndSource);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}
}

LFCore_API void LFTransactionDelete(LFTransactionList* tl, bool PutInTrash)
{
	assert(tl);

	// Reset
	tl->Reset();

	// Process
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
			if ((tl->m_Items[a].Item->Type & LFTypeMask)==LFTypeFile)
			{
				CIndex* idx1;
				CIndex* idx2;
				HANDLE StoreLock = NULL;
				unsigned int res = OpenStore(tl->m_Items[a].Item->StoreID, true, idx1, idx2, NULL, &StoreLock);

				if (res==LFOk)
				{
					if (idx1)
					{
						idx1->Delete(tl, PutInTrash);
						delete idx1;
					}
					if (idx2)
					{
						idx2->Delete(tl, PutInTrash);
						delete idx2;
					}

					ReleaseMutexForStore(StoreLock);
				}
				else
				{
					tl->SetError(tl->m_Items[a].Item->StoreID, res);
				}
			}
			else
			{
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = true;
			}
}

LFCore_API void LFTransactionResolvePhysicalLocations(LFTransactionList* tl, bool IncludePIDL)
{
	// Reset
	tl->Reset();

	// Retrieve physical paths
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
			if ((tl->m_Items[a].Item->Type & LFTypeMask)==LFTypeFile)
			{
				CIndex* idx1;
				CIndex* idx2;
				HANDLE StoreLock = NULL;
				unsigned int res = OpenStore(tl->m_Items[a].Item->StoreID, false, idx1, idx2, NULL, &StoreLock);

				if (res==LFOk)
				{
					if (idx1)
					{
						idx1->ResolvePhysicalLocations(tl);
						delete idx1;
					}
					if (idx2)
						delete idx2;

					ReleaseMutexForStore(StoreLock);
				}
				else
				{
					tl->SetError(tl->m_Items[a].Item->StoreID, res);
				}
			}
			else
			{
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = true;
			}

	// PIDLs
	if (IncludePIDL)
	{
		IShellFolder* pDesktop = NULL;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
		{
			for (unsigned int a=0; a<tl->m_ItemCount; a++)
				if ((tl->m_Items[a].Processed) && (tl->m_Items[a].LastError==LFOk))
					if (FAILED(pDesktop->ParseDisplayName(NULL, NULL, &tl->m_Items[a].Path[4], NULL, &tl->m_Items[a].pidlFQ, NULL)))
						tl->m_Items[a].LastError = tl->m_LastError = LFIllegalPhysicalPath;

			pDesktop->Release();
		}
	}

	tl->m_Resolved = true;
}

LFCore_API unsigned int LFTransactionRename(char* StoreID, char* FileID, wchar_t* NewName)
{
	assert(StoreID);
	assert(FileID);
	assert(NewName);

	CIndex* idx1;
	CIndex* idx2;
	HANDLE StoreLock = NULL;
	unsigned int res = OpenStore(StoreID, true, idx1, idx2, NULL, &StoreLock);

	if (res==LFOk)
	{
		if (idx1)
		{
			res = idx1->Rename(FileID, NewName);
			delete idx1;
		}
		if (idx2)
		{
			if (res==LFOk)
				res = idx2->Rename(FileID, NewName);
			delete idx2;
		}

		ReleaseMutexForStore(StoreLock);
	}

	return res;
}


LFCore_API void LFTransactionDelete(LFFileIDList* il, bool PutInTrash)
{
	assert(il);

	// Reset
	il->Reset();

	// Process
	for (unsigned int a=0; a<il->m_ItemCount; a++)
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			CIndex* idx1;
			CIndex* idx2;
			HANDLE StoreLock = NULL;
			unsigned int res = OpenStore(il->m_Items[a].StoreID, true, idx1, idx2, NULL, &StoreLock);

			if (res==LFOk)
			{
				if (idx1)
				{
					idx1->Delete(il, PutInTrash);
					delete idx1;
				}
				if (idx2)
				{
					idx2->Delete(il, PutInTrash);
					delete idx2;
				}

				ReleaseMutexForStore(StoreLock);
			}
			else
			{
				il->SetError(il->m_Items[a].StoreID, res);
			}
		}
}

LFCore_API void LFTransactionAddToSearchResult(LFFileIDList* il, LFSearchResult* res)
{
	assert(il);

	LFSearchResult* sr = res;

	// Reset
	il->Reset();

	// Process
	for (unsigned int a=0; a<il->m_ItemCount; a++)
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			CIndex* idx1;
			CIndex* idx2;
			HANDLE StoreLock = NULL;
			unsigned int res = OpenStore(il->m_Items[a].StoreID, false, idx1, idx2, NULL, &StoreLock);

			if (res==LFOk)
			{
				if (idx1)
				{
					idx1->AddToSearchResult(il, sr);
					delete idx1;
				}
				if (idx2)
					delete idx2;

				ReleaseMutexForStore(StoreLock);
			}
			else
			{
				il->SetError(il->m_Items[a].StoreID, res);
			}
		}
}
