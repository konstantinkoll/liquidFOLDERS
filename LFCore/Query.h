#pragma once
#include "liquidFOLDERS.h"

int PassesFilterCore(LFItemDescriptor* i, LFFilter* filter);
bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* filter);
LFSearchResult* QueryDomains(LFFilter* f);
