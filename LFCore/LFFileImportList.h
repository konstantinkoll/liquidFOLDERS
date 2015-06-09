#pragma once
#include "LFDynArray.h"

struct LFFIL2_Item
{
	wchar_t Path[MAX_PATH];
	unsigned int LastError;
	bool Processed;
};


class LFFileImportList : public LFDynArray<LFFIL2_Item>
{
public:
	LFFileImportList();

	bool AddPath(wchar_t* path);
	void Resolve(bool recursive);
	void SetError(unsigned int idx, unsigned int res, LFProgress* pProgress=NULL);

	unsigned int m_FileCount;
	__int64 m_FileSize;
};
