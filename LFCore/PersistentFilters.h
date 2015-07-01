#pragma once
#include "LF.h"

#pragma pack(push,1)

struct PersistentFilterHeader
{
	CHAR ID[9];					// LFFilter
	BYTE Version;				// Should be 1 across all platforms
	UINT szHeader;
	UINT szBody;
	UINT szCondition;
};

struct PersistentFilterBody
{
	BOOL AllStores;
	WCHAR Searchterm[256];
	UINT cConditions;
};

#pragma pack(pop)

typedef LFFilterCondition PersistentFilterCondition;
