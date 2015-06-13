#pragma once
#include "LF.h"

size_t GetAttributeMaxCharacterCount(unsigned int attr);
size_t GetAttributeSize(unsigned int attr, const void* v);
void FreeAttribute(LFItemDescriptor* i, unsigned int attr);
void SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v);
LFItemDescriptor* AllocFolderDescriptor();
