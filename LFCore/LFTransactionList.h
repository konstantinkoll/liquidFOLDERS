
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

class LFTransactionList : public LFDynArray<LFTransactionListItem>
{
public:
	LFTransactionList();
	~LFTransactionList();

	BOOL AddItem(LFItemDescriptor* pItemDescriptor, UINT_PTR UserData=0);
	BOOL AddItem(CHAR* StoreID, CHAR* FileID, LFItemDescriptor* pItemDescriptor=NULL, UINT_PTR UserData=0);
	void SetError(CHAR* StoreID, UINT Result, LFProgress* pProgress=NULL);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	HGLOBAL CreateDropFiles();
	HGLOBAL CreateLiquidFiles();
	void DoTransaction(UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, LFVariantData* pVariantData1=NULL, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL);

	BOOL m_Modified;
	BOOL m_Resolved;

private:
	BOOL SetStoreAttributes(LFVariantData* pVariantData, WCHAR** ppStoreName, WCHAR** ppStoreComments);
};
