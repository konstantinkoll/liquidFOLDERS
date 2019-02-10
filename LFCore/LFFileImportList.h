
#pragma once
#include "LFDynArray.h"


#define ALWAYSFORBIDDEN     (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL | FILE_ATTRIBUTE_SYSTEM)

struct LFFileImportItem
{
	WCHAR Path[MAX_PATH];
	UINT LastError;
	BOOL Processed;
	WIN32_FIND_DATA FindData;
	BOOL FindDataPresent;
};

class LFFileImportList sealed : public LFDynArray<LFFileImportItem, 128, 128>
{
public:
	LFFileImportList();

	BOOL AddPath(LPCWSTR pPath, WIN32_FIND_DATA* pFindData=NULL);
	void Resolve(BOOL Recursive, LFProgress* pProgress=NULL);
	void SortItems();
	LPCWSTR GetFileName(UINT Index);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	UINT DoFileImport(BOOL Recursive, const STOREID& StoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress=NULL);

	UINT m_LastError;

private:
	static BOOL IsPathGUID(LPCWSTR pPath);
	static BOOL IsPathEligible(SIZE_T szPath, const WIN32_FIND_DATA& FindData, DWORD Forbidden=ALWAYSFORBIDDEN);
	static INT __stdcall CompareItems(LFFileImportItem* pData1, LFFileImportItem* pData2, const SortParameters& Parameters);
};

inline void LFFileImportList::SortItems()
{
	LFDynArray::SortItems((PFNCOMPARE)CompareItems);
}
