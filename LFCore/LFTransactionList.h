#pragma once
#include "LF.h"
#include "LFDynArray.h"
#include <shlobj.h>

struct LFTL_Item
{
	LFItemDescriptor* Item;
	UINT UserData;

	UINT LastError;
	BOOL Processed;

	WCHAR Path[2*MAX_PATH];
	LPITEMIDLIST pidlFQ;
};


class LFTransactionList : public LFDynArray<LFTL_Item>
{
public:
	LFTransactionList();
	~LFTransactionList();

	BOOL AddItemDescriptor(LFItemDescriptor* i, UINT UserData);
	void Reset();
	void SetError(CHAR* key, UINT error, LFProgress* pProgress=NULL);
	void SetError(UINT idx, UINT Result, LFProgress* pProgress=NULL);
	LPITEMIDLIST DetachPIDL(UINT idx);
	HGLOBAL CreateDropFiles();
	HGLOBAL CreateLiquidFiles();

	BOOL m_Changes;
	BOOL m_Resolved;
};
