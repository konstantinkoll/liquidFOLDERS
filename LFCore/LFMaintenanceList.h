
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

class LFMaintenanceList : public LFDynArray<LFMaintenanceListItem, 16, 16>
{
public:
	LFMaintenanceList();

	BOOL AddItem(LFStoreDescriptor* pStoreDescriptor, UINT Result);

	UINT m_LastError;
};
