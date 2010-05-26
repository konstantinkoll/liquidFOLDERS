#pragma once
#include "liquidFOLDERS.h"

void SetFileDomainAndSlave(LFItemDescriptor* i);
LFItemDescriptor* GetItemDescriptorForFile(wchar_t* fn, LFItemDescriptor* i=NULL);
