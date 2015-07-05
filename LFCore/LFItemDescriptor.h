#pragma once
#include "LF.h"

SIZE_T GetAttributeMaxCharacterCount(UINT Attr);
SIZE_T GetAttributeSize(UINT Attr, const void* v);
void FreeAttribute(LFItemDescriptor* i, UINT Attr);
void SetAttribute(LFItemDescriptor* i, UINT Attr, const void* v);
LFItemDescriptor* AllocFolderDescriptor();
