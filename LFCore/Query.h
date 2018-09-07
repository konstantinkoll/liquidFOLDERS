
#pragma once
#include "LF.h"


#define QUERYSTATE_SEARCHTERMMASK          0x03
#define QUERYSTATE_SEARCHTERM_UNKNOWN      0x00
#define QUERYSTATE_SEARCHTERM_NOLETTERS    0x01
#define QUERYSTATE_SEARCHTERM_LETTERS      0x02

#define QUERYSTATE_PASSEDMASK              0x1C
#define QUERYSTATE_PASSED_MASTER           0x04
#define QUERYSTATE_PASSED_SLAVE            0x08
#define QUERYSTATE_PASSED_SEARCHTERM       0x10

BOOL PassesFilter(UINT TableID, LPCVOID pTableData, LFFilter* pFilter, BYTE& QueryState);
BOOL PassesFilter(LFItemDescriptor* pItemDescriptor, LFFilter* pFilter, BYTE& QueryState);

inline void InitializeQueryState(BYTE& QueryState)
{
	QueryState = 0;
}

inline void ResetQueryState(BYTE& QueryState)
{
	QueryState &= QUERYSTATE_SEARCHTERMMASK;
}
