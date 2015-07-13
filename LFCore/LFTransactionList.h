
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

	BOOL m_Modified;
	BOOL m_Resolved;
};
