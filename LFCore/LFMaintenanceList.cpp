#include "stdafx.h"
#include "LFMaintenanceList.h"
#include <assert.h>


LFMaintenanceList::LFMaintenanceList()
	: LFDynArray()
{
}

BOOL LFMaintenanceList::AddStore(UINT _Result, WCHAR* _Name, CHAR* _StoreID, UINT _Icon)
{
	assert(_Name);
	assert(_StoreID);

	LFML_Item item = { _Result, L"", "", _Icon };
	wcscpy_s(item.Name, 256, _Name);
	strcpy_s(item.StoreID, LFKeySize, _StoreID);

	return LFDynArray::AddItem(item);
}
