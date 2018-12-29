
#pragma once
#include "LF.h"
#include "LFDynArray.h"


struct LFMaintenanceListItem
{
	WCHAR Name[256];
	WCHAR Comments[256];
	ABSOLUTESTOREID StoreID;
	UINT Result;
	UINT IconID;
};

class LFMaintenanceList sealed : public LFDynArray<LFMaintenanceListItem, 16, 16>
{
public:
	LFMaintenanceList();

	BOOL AddItem(LFStoreDescriptor* pStoreDescriptor, UINT Result);
	void SortItems();

	UINT m_LastError;

private:
	static INT __stdcall CompareItems(const LFMaintenanceListItem* pData1, const LFMaintenanceListItem* pData2, const SortParameters& Parameters);
};

inline void LFMaintenanceList::SortItems()
{
	LFDynArray::SortItems((PFNCOMPARE)CompareItems);
}
