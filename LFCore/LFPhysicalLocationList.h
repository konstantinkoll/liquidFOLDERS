#pragma once
#include "DynArray.h"
#include "PIDL.h"

struct LFPLL_Item
{
	char StoreID[LFKeySize];
	char FileID[LFKeySize];
	wchar_t FileName[256];
	unsigned int LastError;
	bool Processed;
	wchar_t Path[2*MAX_PATH];
	LPITEMIDLIST pidlFQ;
};


class LFPhysicalLocationList : public DynArray<LFPLL_Item>
{
public:
	LFPhysicalLocationList();
	~LFPhysicalLocationList();

	bool AddItemDescriptor(LFItemDescriptor* i);
	void SetError(char* key, unsigned int error);
	void Resolve(bool IncludePIDL=false);
	LPITEMIDLIST DetachPIDL(unsigned int idx);
	HGLOBAL CreateDropFiles();
};
