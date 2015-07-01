#pragma once
#include "LF.h"

BYTE GetRatingCategory(const BYTE rating);
UINT GetSizeCategory(const INT64 sz);
UINT GetDurationCategory(const UINT duration);
void GetServer(CHAR* URL, CHAR* Server);
INT PassesFilterCore(LFCoreAttributes* ca, LFFilter* f);
BOOL PassesFilterSlaves(LFItemDescriptor* i, LFFilter* f);
LFSearchResult* QueryDomains(LFFilter* f);
