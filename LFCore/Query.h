#pragma once
#include "liquidFOLDERS.h"

unsigned char GetRatingCategory(const unsigned char rating);
unsigned int GetSizeCategory(const __int64 sz);
unsigned int GetDurationCategory(const unsigned int duration);
bool GetNamePrefix(wchar_t* FullName, wchar_t* Buffer);
void GetServer(char* URL, char* Server);
int PassesFilterCore(LFCoreAttributes* ca, LFFilter* filter);
bool PassesFilterSlaves(LFItemDescriptor* i, LFFilter* filter);
LFSearchResult* QueryDomains(LFFilter* f);
