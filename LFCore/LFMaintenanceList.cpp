
#include "stdafx.h"
#include "LFCore.h"
#include "LFMaintenanceList.h"
#include <assert.h>


LFCORE_API void LFFreeMaintenanceList(LFMaintenanceList* pMaintenanceList)
{
	assert(pMaintenanceList);

	delete pMaintenanceList;
}


// LFMaintenanceList
//

LFMaintenanceList::LFMaintenanceList()
	: LFDynArray()
{
	m_LastError = LFOk;
}

BOOL LFMaintenanceList::AddItem(LPCWSTR Name, LPCWSTR Comments, LPCSTR StoreID, UINT Result, UINT Icon)
{
	assert(Name);
	assert(Comments);
	assert(StoreID);

	LFMaintenanceListItem Item;

	wcscpy_s(Item.Name, 256, Name);
	wcscpy_s(Item.Comments, 256, Comments);
	strcpy_s(Item.StoreID, LFKeySize, StoreID);
	Item.Result = Result;
	Item.Icon = Icon;

	return LFDynArray::AddItem(Item);
}
