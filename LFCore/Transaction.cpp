#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFVariantData.h"
#include "Transaction.h"


extern LFMessageIDs LFMessages;

void UpdateStore(LFTransactionList* tl, unsigned int idx, LFVariantData* value, wchar_t* ustr, bool& Updated)
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
		LFSetAttributeVariantData(tl->m_Entries[idx].Item, value, ustr);
		tl->m_Changes = true;
		Updated |= true;
	}
	else
	{
		tl->m_LastError = tl->m_Entries[idx].LastError = result;
	}
}

LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, wchar_t* ustr1, LFVariantData* value2, wchar_t* ustr2, LFVariantData* value3, wchar_t* ustr3)
{
	bool StoresUpdated = false;

	// Stores and illegal item types
	for (unsigned int a=0; a<tl->m_Count; a++)
		if (tl->m_Entries[a].LastError==LFOk)
			switch (tl->m_Entries[a].Item->Type & LFTypeMask)
			{
			case LFTypeStore:
				UpdateStore(tl, a, value1, ustr1, StoresUpdated);
				if (value2)
					UpdateStore(tl, a, value2, ustr2, StoresUpdated);
				if (value3)
					UpdateStore(tl, a, value3, ustr3, StoresUpdated);
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
