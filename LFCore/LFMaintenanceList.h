
#pragma once
#include "LF.h"
#include "LFDynArray.h"


struct LFMaintenanceListItem
{
	WCHAR Name[256];
	WCHAR Comments[256];
	CHAR StoreID[LFKeySize];
	UINT Result;
	UINT IconID;
};

class LFMaintenanceList : public LFDynArray<LFMaintenanceListItem, 16, 16>
{
public:
	LFMaintenanceList();

	BOOL AddItem(LPCWSTR Name, LPCWSTR Comments, LPCSTR StoreID, UINT Result, UINT IconID);

	UINT m_LastError;
};
