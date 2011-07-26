#pragma once
#include "DynArray.h"

struct LFFIL2_Item
{
	wchar_t Path[MAX_PATH];
	unsigned int LastError;
	bool Processed;
};


class LFFileImportList : public DynArray<LFFIL2_Item>
{
public:
	LFFileImportList();

	bool AddPath(wchar_t* path);
	void Resolve(bool recursive);

	unsigned int m_FileCount;
	__int64 m_FileSize;
};
