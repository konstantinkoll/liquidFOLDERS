#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFVariantData.h"
#include "Transaction.h"


extern LFMessageIDs LFMessages;

void UpdateStore(LFTransactionList* tl, unsigned int idx, LFVariantData* value, bool& Updated)
{
	unsigned int result = LFIllegalAttribute;

	switch (value->Attr)
	{
	case LFAttrFileName:
		result = LFSetStoreAttributes(tl->m_Entries[idx].Item->CoreAttributes.StoreID, value->UnicodeString, NULL, NULL, true);
		break;
	case LFAttrComment:
		result = LFSetStoreAttributes(tl->m_Entries[idx].Item->CoreAttributes.StoreID, NULL, value->UnicodeString, NULL, true);
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
}

LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	bool StoresUpdated = false;

	// Stores and illegal item types
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
				break;
			default:
				tl->m_LastError = tl->m_Entries[a].LastError = LFIllegalItemType;
			}

	if (StoresUpdated)
		SendNotifyMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores, (LPARAM)hWndSource);

	// Files
	//TODO
}
