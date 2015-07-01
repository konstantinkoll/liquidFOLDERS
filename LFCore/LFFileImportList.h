#pragma once
#include "LFDynArray.h"

struct LFFIL2_Item
{
	WCHAR Path[MAX_PATH];
	UINT LastError;
	BOOL Processed;
};


class LFFileImportList : public LFDynArray<LFFIL2_Item>
{
public:
	LFFileImportList();

	BOOL AddPath(WCHAR* path);
	void Resolve(BOOL recursive);
	void SetError(UINT idx, UINT Result, LFProgress* pProgress=NULL);

	UINT m_FileCount;
	INT64 m_FileSize;
};
