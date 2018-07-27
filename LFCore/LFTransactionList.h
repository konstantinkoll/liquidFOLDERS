
#pragma once
#include "LF.h"
#include "LFDynArray.h"
#include <shlobj.h>


#pragma warning(push)
#pragma warning(disable: 4201)

struct LFTransactionListItem
{
	union
	{
		LIQUIDFILEITEM FileItem;

		struct
		{
			ABSOLUTESTOREID StoreID;
			FILEID FileID;
		};
	};

	LFItemDescriptor* pItemDescriptor;
	UINT_PTR UserData;

	UINT LastError;
	BOOL Processed;

	WCHAR Path[2*MAX_PATH];
	LPITEMIDLIST pidlFQ;
};

#pragma warning(pop)

class LFTransactionList : public LFDynArray<LFTransactionListItem, 128, 128>
{
public:
	LFTransactionList();
	~LFTransactionList();

	BOOL AddItem(LFItemDescriptor* pItemDescriptor, UINT_PTR UserData=0);
	BOOL AddItem(const LIQUIDFILEITEM& FileItem);
	void SetError(const ABSOLUTESTOREID& AbsoluteStoreID, UINT Result, LFProgress* pProgress=NULL);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	HGLOBAL CreateDropFiles();
	HGLOBAL CreateLiquidFiles();
	UINT DoTransaction(UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, const LFVariantData* pVariantData1=NULL, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL);

	UINT m_LastError;
	BOOL m_Modified;
	BOOL m_Resolved;

protected:
	BOOL AddItem(const ABSOLUTESTOREID& AbsoluteStoreID, const FILEID& FileID, LFItemDescriptor* pItemDescriptor=NULL, UINT_PTR UserData=0);

private:
	BOOL SetStoreAttributes(const LFVariantData* pVData, LPCWSTR* ppStoreName, LPCWSTR* ppStoreComments);
};

inline BOOL LFTransactionList::AddItem(LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	assert(pItemDescriptor);

	return AddItem(pItemDescriptor->StoreID, pItemDescriptor->CoreAttributes.FileID, pItemDescriptor, UserData);
}

inline BOOL LFTransactionList::AddItem(const LIQUIDFILEITEM& FileItem)
{
	return AddItem(FileItem.StoreID, FileItem.FileID);
}
