
#pragma once
#include "LF.h"


SIZE_T GetAttributeSize(UINT Attr, LPCVOID Value);
void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, LPCVOID Value);

LFItemDescriptor* AllocFolderDescriptor(UINT Attr, const LFFileSummary& FileSummary, INT FirstAggregate=-1, INT LastAggregate=-1);
void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, LPVOID pSlaveData);
