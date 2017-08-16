
#pragma once
#include "LF.h"


SIZE_T GetAttributeSize(UINT Attr, LPCVOID pValue);
void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, LPCVOID pValue);

UINT GetColoredFolderIconID(const LFFileSummary& FileSummary);

LFItemDescriptor* AllocFolderDescriptor(UINT Attr, const LFFileSummary& FileSummary, INT AggregateFirst=-1, INT AggregateLast=-1);
void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, LPVOID pSlaveData);
