#pragma once
#include "liquidFOLDERS.h"

unsigned char GetRatingCategory(const unsigned char rating);
unsigned int GetSizeCategory(const __int64 sz);
int PassesFilterCore(LFCoreAttributes* ca, LFFilter* filter);
bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* filter);
LFSearchResult* QueryDomains(LFFilter* f);
