#pragma once
#include "liquidFOLDERS.h"

int PassesFilterCore(LFCoreAttributes* ca, LFFilter* filter);
bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* filter);
LFSearchResult* QueryDomains(LFFilter* f);
