#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "Stores.h"
#include "Transaction.h"


extern LFMessageIDs LFMessages;

void UpdateStore(LFTransactionList* tl, unsigned int idx, LFVariantData* value, bool& Updated)
{
	unsigned int result = LFIllegalAttribute;

	switch (value->Attr)
	{
	case LFAttrFileName:
		result = LFSetStoreAttributes(tl->m_Entries[idx].Item->StoreID, value->UnicodeString, NULL, NULL, true);
		break;
	case LFAttrComment:
		result = LFSetStoreAttributes(tl->m_Entries[idx].Item->StoreID, NULL, value->UnicodeString, NULL, true);
		break;
	default:
		result = LFIllegalAttribute;
	}

	if (result==LFOk)
	{
		LFSetAttributeVariantData(tl->m_Entries[idx].Item, value);
		tl->m_Changes = true;
		Updated |= true;
	}
	else
	{
		tl->m_LastError = tl->m_Entries[idx].LastError = result;
	}
	tl->m_Entries[idx].Processed = true;
}

LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	bool StoresUpdated = false;

	// Reset processed flag
	for (unsigned int a=0; a<tl->m_Count; a++)
		tl->m_Entries[a].Processed = false;

	// Process
	for (unsigned int a=0; a<tl->m_Count; a++)
		if (tl->m_Entries[a].LastError==LFOk)
			switch (tl->m_Entries[a].Item->Type & LFTypeMask)
			{
			case LFTypeStore:
				UpdateStore(tl, a, value1, StoresUpdated);
				if (value2)
					UpdateStore(tl, a, value2, StoresUpdated);
				if (value3)
					UpdateStore(tl, a, value3, StoresUpdated);
			case LFTypeFile:
				if (!tl->m_Entries[a].Processed)
				{
					CIndex* idx1;
					CIndex* idx2;
					HANDLE StoreLock = NULL;
					unsigned int res = OpenStore(tl->m_Entries->Item->StoreID, true, idx1, idx2, NULL, &StoreLock);

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
						for (unsigned int b=a; b<tl->m_Count; b++)
						{
							LFItemDescriptor* i = tl->m_Entries[b].Item;
							if ((i->Type & LFTypeMask)==LFTypeFile)
								if ((strcmp(i->StoreID, tl->m_Entries[a].Item->StoreID)==0) && (!tl->m_Entries[b].Processed))
								{
									tl->m_Entries[b].LastError = tl->m_LastError = res;
									tl->m_Entries[b].Processed = true;
								}
						}
					}
				}

				break;
			default:
				tl->m_LastError = tl->m_Entries[a].LastError = LFIllegalItemType;
				tl->m_Entries[a].Processed = true;
			}

	// Update messages
	if (StoresUpdated)
		SendStoreNotifyMessage(LFMessages.StoreAttributesChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores, hWndSource);
}

LFCore_API void LFTransactionDelete(LFTransactionList* tl)
{
	// Reset processed flag
	for (unsigned int a=0; a<tl->m_Count; a++)
		tl->m_Entries[a].Processed = false;

	// Process
	for (unsigned int a=0; a<tl->m_Count; a++)
		if ((tl->m_Entries[a].LastError==LFOk) && (!tl->m_Entries[a].Processed))
			if ((tl->m_Entries[a].Item->Type & LFTypeMask)==LFTypeFile)
			{
				CIndex* idx1;
				CIndex* idx2;
				LFStoreDescriptor* slot;
				HANDLE StoreLock = NULL;
				unsigned int res = OpenStore(tl->m_Entries->Item->StoreID, true, idx1, idx2, &slot, &StoreLock);

				if (res==LFOk)
				{
					if (idx1)
					{
						idx1->Delete(tl, &slot->DatPath[0]);
						delete idx1;
					}
					if (idx2)
					{
						idx2->Delete(tl, &slot->DatPath[0]);
						delete idx2;
					}

					ReleaseMutexForStore(StoreLock);
				}
				else
				{
					// Cannot open index, so mark all subsequent files in the same store as processed
					for (unsigned int b=a; b<tl->m_Count; b++)
					{
						LFItemDescriptor* i = tl->m_Entries[b].Item;
						if ((i->Type & LFTypeMask)==LFTypeFile)
							if ((strcmp(i->StoreID, tl->m_Entries[a].Item->StoreID)==0) && (!tl->m_Entries[b].Processed))
							{
								tl->m_Entries[b].LastError = tl->m_LastError = res;
								tl->m_Entries[b].Processed = true;
							}
					}
				}
			}
			else
			{
				tl->m_LastError = tl->m_Entries[a].LastError = LFIllegalItemType;
				tl->m_Entries[a].Processed = true;
			}
}
