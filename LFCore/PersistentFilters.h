#pragma once
#include "liquidFOLDERS.h"

#pragma pack(push)
#pragma pack(1)

struct PersistentFilterHeader
{
	char ID[9];					// LFFilter
	unsigned char Version;		// Should be 1 across all platforms
	unsigned int szHeader;
	unsigned int szBody;
	unsigned int szCondition;
};

#pragma pack(pop)

struct PersistentFilterBody
{
	char StoreID[LFKeySize];
	wchar_t Searchterm[256];
	unsigned int cConditions;
};

typedef LFFilterCondition PersistentFilterCondition;
