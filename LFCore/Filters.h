
#pragma once
#include "LF.h"


#pragma pack(push,1)

struct LFPersistentFilterHeader
{
	CHAR ID[9];					// LFFilter
	BYTE Version;				// Should be 1 across all platforms
	BYTE Reserved[8];
	UINT szCondition;
	BOOL AllStores;
	WCHAR SearchTerm[256];
	UINT cConditions;
};

#pragma pack(pop)

typedef LFFilterCondition LFPersistentFilterCondition;


LFFilter* CloneFilter(const LFFilter* pFilter);
BOOL StoreFilter(LPCWSTR pPath, LFFilter* pFilter);
