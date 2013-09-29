#pragma once
#include "liquidFOLDERS.h"

size_t GetAttributeMaxCharacterCount(unsigned int attr);
size_t GetAttributeSize(unsigned int attr, const void* v);
void LFCore_API SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v);
LFItemDescriptor* AllocFolderDescriptor();
