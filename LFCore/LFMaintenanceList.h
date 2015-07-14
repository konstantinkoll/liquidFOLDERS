
#pragma once
#include "LF.h"
#include "LFDynArray.h"


struct LFMaintenaceListItem
{
	WCHAR Name[256];
	WCHAR Comments[256];
	CHAR StoreID[LFKeySize];
	UINT Result;
	UINT Icon;
};

class LFMaintenanceList : public LFDynArray<LFMaintenaceListItem>
{
public:
	BOOL AddItem(WCHAR* Name, WCHAR* Comments, CHAR* StoreID, UINT Result, UINT Icon);
};
