#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"
#include <shlobj.h>

struct LFTL_Item
{
	LFItemDescriptor* Item;
	unsigned int UserData;

	unsigned int LastError;
	bool Processed;

	wchar_t Path[2*MAX_PATH];
	LPITEMIDLIST pidlFQ;
};


class LFTransactionList : public DynArray<LFTL_Item>
{
public:
	LFTransactionList();
	~LFTransactionList();

	bool AddItemDescriptor(LFItemDescriptor* i, unsigned int UserData);
	void Reset();
	void SetError(char* key, unsigned int error, LFProgress* pProgress=NULL);
	void SetError(unsigned int idx, unsigned int res, LFProgress* pProgress=NULL);
	LPITEMIDLIST DetachPIDL(unsigned int idx);
	HGLOBAL CreateDropFiles();
	HGLOBAL CreateLiquidFiles();

	bool m_Changes;
	bool m_Resolved;
};
