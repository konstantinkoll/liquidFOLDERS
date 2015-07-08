
#pragma once
#include "LF.h"


BOOL PassesFilter(UINT TableID, void* pTableData, LFFilter* pFilter, BOOL& CheckSearchterm);
BOOL PassesFilter(LFItemDescriptor* i, LFFilter* pFilter);
