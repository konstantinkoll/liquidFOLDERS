
#pragma once
#include "LF.h"


SIZE_T GetAttributeSize(UINT Attr, LPCVOID Value);
void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, LPCVOID Value);

LFItemDescriptor* AllocFolderDescriptor();
void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, LPVOID pSlaveData);
