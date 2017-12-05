
#pragma once
#include "LF.h"
#include "LFDynArray.h"
#include <shlobj.h>


struct LFTransactionListItem
{
	CHAR StoreID[LFKeySize];
	CHAR FileID[LFKeySize];
	LFItemDescriptor* pItemDescriptor;
	UINT_PTR UserData;

	UINT LastError;
	BOOL Processed;

	WCHAR Path[2*MAX_PATH];
	LPITEMIDLIST pidlFQ;
};

class LFTransactionList : public LFDynArray<LFTransactionListItem, 128, 128>
{
public:
	LFTransactionList();
	~LFTransactionList();

	BOOL AddItem(LFItemDescriptor* pItemDescriptor, UINT_PTR UserData=0);
	BOOL AddItem(LPCSTR pStoreID, LPCSTR pFileID, LFItemDescriptor* pItemDescriptor=NULL, UINT_PTR UserData=0);
	void SetError(LPCSTR pStoreID, UINT Result, LFProgress* pProgress=NULL);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	HGLOBAL CreateDropFiles();
	HGLOBAL CreateLiquidFiles();
	void DoTransaction(UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, const LFVariantData* pVariantData1=NULL, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL);

	UINT m_LastError;
	BOOL m_Modified;
	BOOL m_Resolved;

private:
	BOOL SetStoreAttributes(const LFVariantData* pVData, LPCWSTR* ppStoreName, LPCWSTR* ppStoreComments);
};
