#pragma once
#include "DynArray.h"


class LFFileImportList : public DynArray<wchar_t*>
{
public:
	LFFileImportList();
	~LFFileImportList();

	bool AddPath(wchar_t* path);
	void Resolve(bool recursive);

	unsigned int m_FileCount;
	__int64 m_FileSize;
};
