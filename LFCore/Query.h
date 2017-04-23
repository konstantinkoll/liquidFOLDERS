
#pragma once
#include "LF.h"


BOOL PassesFilter(UINT TableID, LPVOID pTableData, LFFilter* pFilter, BOOL& CheckSearchterm, BYTE& SearchtermContainsLetters);
BOOL PassesFilter(LFItemDescriptor* pItemDescriptor, LFFilter* pFilter);
