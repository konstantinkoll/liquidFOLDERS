#pragma once
#include "liquidFOLDERS.h"

struct PersistentFilterHeader
{
	char ID[9];					// LFFilter
	unsigned char Version;		// Should be 1 across all platforms
	unsigned int szHeader;
	unsigned int szBody;
	unsigned int szCondition;
};

struct PersistentFilterBody
{
	char StoreID[LFKeySize];
	wchar_t Searchterm[256];
	unsigned int cConditions;
};

typedef LFFilterCondition PersistentFilterCondition;
