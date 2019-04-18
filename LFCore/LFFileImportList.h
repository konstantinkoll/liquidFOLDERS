
#pragma once
#include "LFDynArray.h"


#define ALWAYSFORBIDDEN     (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL | FILE_ATTRIBUTE_SYSTEM)

#define PROCESSED_UNTOUCHED    0x00
#define PROCESSED_FINISHED     0xFF

#define FII_FINDDATAVALID       0x01
#define FII_MATCHED             0x02

struct LFFileImportItem
{
	WCHAR Path[MAX_PATH];
	WIN32_FIND_DATA FindData;
	UINT LastError;
	BYTE Processed;
	BYTE Flags;
};

class LFFileImportList sealed : public LFDynArray<LFFileImportItem, 128, 128>
{
public:
	LFFileImportList();

	BOOL AddPath(LPCWSTR pPath);
	BOOL AddPath(LPCWSTR pPath, const WIN32_FIND_DATA& FindData);
	UINT Resolve(BOOL Recursive, LFProgress* pProgress=NULL);
	LPCWSTR GetFileName(UINT Index);
	void SetError(UINT Index, UINT Result, LFProgress* pProgress=NULL);
	UINT DoFileImport(BOOL Recursive, const STOREID& StoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress=NULL);
	LFFileImportItem* FindPath(LPCWSTR pPath);

	UINT m_LastError;

private:
	static BOOL IsPathGUID(LPCWSTR pPath);
	static BOOL IsPathEligible(SIZE_T szPath, const WIN32_FIND_DATA& FindData, DWORD Forbidden=ALWAYSFORBIDDEN);
	static INT __stdcall CompareItems(LFFileImportItem* pData1, LFFileImportItem* pData2, const SortParameters& Parameters);
	void SortItems();

	BOOL m_IsSorted;
};

inline void LFFileImportList::SortItems()
{
	LFDynArray::SortItems((PFNCOMPARE)CompareItems);
}
