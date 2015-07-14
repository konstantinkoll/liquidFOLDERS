
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

BOOL LFMaintenanceList::AddItem(WCHAR* Name, WCHAR* Comments, CHAR* StoreID, UINT Result, UINT Icon)
{
	assert(Name);
	assert(Comments);
	assert(StoreID);

	LFMaintenaceListItem Item;

	wcscpy_s(Item.Name, 256, Name);
	wcscpy_s(Item.Comments, 256, Comments);
	strcpy_s(Item.StoreID, LFKeySize, StoreID);
	Item.Result = Result;
	Item.Icon = Icon;

	return LFDynArray::AddItem(Item);
}
