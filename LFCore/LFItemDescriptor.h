
#pragma once
#include "LF.h"


SIZE_T GetAttributeSize(UINT Attr, LPCVOID pValue);
void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, LPCVOID pValue);

UINT GetColoredFolderIcon(const LFFileSummary& FileSummary);
UINT GetFolderIcon(const LFFileSummary& FileSummary, const LFVariantData& VData, BOOL IgnoreDefaultIcon=FALSE);

LFItemDescriptor* AllocFolderDescriptor(const LFFileSummary& FileSummary, const LFVariantData& VData, LFFilter* pFilter=NULL, INT AggregateFirst=-1, INT AggregateLast=-1);
void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, LPVOID pSlaveData);
