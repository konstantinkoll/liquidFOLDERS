#pragma once
#include "LF.h"
#include "LFDynArray.h"


struct LFML_Item
{
	unsigned int Result;
	wchar_t Name[256];
	char StoreID[LFKeySize];
	unsigned int Icon;
};

class LFMaintenanceList : public LFDynArray<LFML_Item>
{
public:
	LFMaintenanceList();

	bool AddStore(unsigned int _Result, wchar_t* _Name, char* _StoreID, unsigned int _Icon);
};
