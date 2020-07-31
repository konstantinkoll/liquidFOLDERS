
#include "stdafx.h"
#include "LFCore.h"
#include "LFMaintenanceList.h"


// LFMaintenanceList
//

LFMaintenanceList::LFMaintenanceList()
	: LFDynArray()
{
	m_LastError = LFOk;
}

BOOL LFMaintenanceList::AddItem(LFStoreDescriptor* pStoreDescriptor, UINT Result)
{
	assert(pStoreDescriptor);

	LFMaintenanceListItem Item;

	wcscpy_s(Item.Name, 256, pStoreDescriptor->StoreName);
	wcscpy_s(Item.Comments, 256, pStoreDescriptor->Comments);
	Item.StoreID = pStoreDescriptor->StoreID;
	Item.Result = Result;
	Item.IconID = LFGetStoreIcon(*pStoreDescriptor);

	return LFDynArray::AddItem(Item);
}

INT LFMaintenanceList::CompareItems(const LFMaintenanceListItem* pData1, const LFMaintenanceListItem* pData2, const SortParameters& /*Parameters*/)
{
	return (pData1->Result!=pData2->Result) ? (INT)pData2->Result-(INT)pData1->Result : wcscmp(pData1->Name, pData2->Name);
}
