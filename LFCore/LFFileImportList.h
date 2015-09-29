
#pragma once
#include "LFDynArray.h"


struct LFFileImportListItem
{
	WCHAR Path[2*MAX_PATH];
	UINT LastError;
	BOOL Processed;
};

class LFFileImportList : public LFDynArray<LFFileImportListItem>
{
public:
	BOOL AddPath(WCHAR* Path);
	void Resolve(BOOL Recursive, LFProgress* pProgress=NULL);
	WCHAR* GetFileName(UINT Index);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	UINT DoFileImport(BOOL Recursive, CHAR* pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress=NULL);
};
