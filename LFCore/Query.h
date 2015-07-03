
#pragma once
#include "LF.h"


INT PassesFilterCore(LFCoreAttributes* ca, LFFilter* f);
BOOL PassesFilterSlaves(LFItemDescriptor* i, LFFilter* f);
LFSearchResult* QueryDomains(LFFilter* f);
