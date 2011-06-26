
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
	bool StoresUpdated = false;

	// Reset
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
	{
		tl->m_Items[a].LastError = LFOk;
		tl->m_Items[a].Processed = false;
	}

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
						// Cannot open index, so mark all subsequent files in the same store as processed
						for (unsigned int b=a; b<tl->m_ItemCount; b++)
						{
							LFItemDescriptor* i = tl->m_Items[b].Item;
							if ((i->Type & LFTypeMask)==LFTypeFile)
								if ((strcmp(i->StoreID, tl->m_Items[a].Item->StoreID)==0) && (!tl->m_Items[b].Processed))
								{
									tl->m_Items[b].LastError = tl->m_LastError = res;
									tl->m_Items[b].Processed = true;
								}
						}
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

LFCore_API void LFTransactionDelete(LFTransactionList* tl)
{
	assert(tl);

	// Reset
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
	{
		tl->m_Items[a].LastError = LFOk;
		tl->m_Items[a].Processed = false;
	}

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
						idx1->Delete(tl);
						delete idx1;
					}
					if (idx2)
					{
						idx2->Delete(tl);
						delete idx2;
					}

					ReleaseMutexForStore(StoreLock);
				}
				else
				{
					// Cannot open index, so mark all subsequent files in the same store as processed
					for (unsigned int b=a; b<tl->m_ItemCount; b++)
					{
						LFItemDescriptor* i = tl->m_Items[b].Item;
						if ((i->Type & LFTypeMask)==LFTypeFile)
							if ((strcmp(i->StoreID, tl->m_Items[a].Item->StoreID)==0) && (!tl->m_Items[b].Processed))
							{
								tl->m_Items[b].LastError = tl->m_LastError = res;
								tl->m_Items[b].Processed = true;
							}
					}
				}
			}
			else
			{
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = true;
			}
}

LFCore_API void LFTransactionDelete(LFFileIDList* il, bool PutInTrash)
{
	assert(il);

	// Reset
	for (unsigned int a=0; a<il->m_ItemCount; a++)
	{
		il->m_Items[a].LastError = LFOk;
		il->m_Items[a].Processed = false;
	}

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
				// Cannot open index, so mark all subsequent files in the same store as processed
				for (unsigned int b=a; b<il->m_ItemCount; b++)
					if ((strcmp(il->m_Items[b].StoreID, il->m_Items[a].StoreID)==0) && (!il->m_Items[b].Processed))
					{
						il->m_Items[b].LastError = il->m_LastError = res;
						il->m_Items[b].Processed = true;
					}
			}
		}
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
