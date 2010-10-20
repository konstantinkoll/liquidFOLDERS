#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"


struct LFML_Item
{
	unsigned int Result;
	wchar_t Name[256];
	char StoreID[LFKeySize];
	unsigned int Icon;
};

class LFMaintenanceList : public DynArray<LFML_Item>
{
public:
	LFMaintenanceList();

	bool AddStore(unsigned int _Result, wchar_t* _Name, char* _StoreID, unsigned int _Icon);
};
