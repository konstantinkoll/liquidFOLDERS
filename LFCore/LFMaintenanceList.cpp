#include "stdafx.h"
#include "LFMaintenanceList.h"
#include <assert.h>


LFMaintenanceList::LFMaintenanceList()
	: LFDynArray()
{
}

bool LFMaintenanceList::AddStore(unsigned int _Result, wchar_t* _Name, char* _StoreID, unsigned int _Icon)
{
	assert(_Name);
	assert(_StoreID);

	LFML_Item item = { _Result, L"", "", _Icon };
	wcscpy_s(item.Name, 256, _Name);
	strcpy_s(item.StoreID, LFKeySize, _StoreID);

	return LFDynArray::AddItem(item);
}
