
#pragma once
#include "LFDynArray.h"


struct LFFileImportListItem
{
	WCHAR Path[MAX_PATH];
	UINT LastError;
	BOOL Processed;
};

class LFFileImportList : public LFDynArray<LFFileImportListItem>
{
public:
	LFFileImportList();

	BOOL AddPath(WCHAR* Path);
	void Resolve(BOOL Recursive);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);

	UINT m_FileCount;
	INT64 m_FileSize;
};
