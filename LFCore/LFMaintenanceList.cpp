
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
	Item.IconID = LFGetStoreIcon(pStoreDescriptor);

	return LFDynArray::AddItem(Item);
}
