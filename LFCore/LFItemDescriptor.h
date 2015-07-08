
#pragma once
#include "LF.h"


SIZE_T GetAttributeMaxCharacterCount(UINT Attr);
SIZE_T GetAttributeSize(UINT Attr, const void* v);
void SetAttribute(LFItemDescriptor* i, UINT Attr, const void* v);

LFItemDescriptor* AllocFolderDescriptor();
void AttachSlave(LFItemDescriptor* i, BYTE SlaveID, void* pSlaveData);
