#pragma once
#include "LF.h"
#include "LFDynArray.h"


struct LFML_Item
{
	UINT Result;
	WCHAR Name[256];
	CHAR StoreID[LFKeySize];
	UINT Icon;
};

class LFMaintenanceList : public LFDynArray<LFML_Item>
{
public:
	LFMaintenanceList();

	BOOL AddStore(UINT _Result, WCHAR* _Name, CHAR* _StoreID, UINT _Icon);
};
