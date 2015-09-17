
#pragma once
#include "LF.h"


SIZE_T GetAttributeMaxCharacterCount(UINT Attr);
SIZE_T GetAttributeSize(UINT Attr, const void* Value);
void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, const void* Value);

LFItemDescriptor* AllocFolderDescriptor();
void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, void* pSlaveData);
