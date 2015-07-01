#pragma once
#include "LF.h"

size_t GetAttributeMaxCharacterCount(UINT Attr);
size_t GetAttributeSize(UINT Attr, const void* v);
void FreeAttribute(LFItemDescriptor* i, UINT Attr);
void SetAttribute(LFItemDescriptor* i, UINT Attr, const void* v);
LFItemDescriptor* AllocFolderDescriptor();
