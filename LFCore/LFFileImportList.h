
#pragma once
#include "LFDynArray.h"


struct LFFileImportListItem
{
	WCHAR Path[MAX_PATH];
	UINT LastError;
	BOOL Processed;
	WIN32_FIND_DATA FindFileData;
	BOOL FindFileDataPresent;
};

class LFFileImportList : public LFDynArray<LFFileImportListItem, 128, 128>
{
public:
	LFFileImportList();

	BOOL AddPath(WCHAR* Path, WIN32_FIND_DATA* pFindFileData=NULL);
	void Resolve(BOOL Recursive, LFProgress* pProgress=NULL);
	void Sort();
	WCHAR* GetFileName(UINT Index);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	UINT DoFileImport(BOOL Recursive, const CHAR* pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress=NULL);

	UINT m_LastError;

private:
	void Heap(UINT Wurzel, const UINT Anz);
};
