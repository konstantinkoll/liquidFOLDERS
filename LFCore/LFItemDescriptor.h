
#pragma once
#include "LF.h"


SIZE_T GetAttributeSize(ATTRIBUTE Attr, LPCVOID pValue);
void SetAttribute(LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr, LPCVOID pValue);

UINT GetColoredFolderIcon(const LFFileSummary& FileSummary);
UINT GetFolderIcon(const LFFileSummary& FileSummary, const LFVariantData& VData, BOOL IgnoreDefaultIcon=FALSE);

LFItemDescriptor* AllocFolderDescriptor(const LFFileSummary& FileSummary, const LFVariantData& VData, LFFilter* pFilter=NULL, INT AggregateFirst=-1, INT AggregateLast=-1);
void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, LPVOID pSlaveData);

inline void ResetStoreData(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	// It is sufficient for now to delete the first two bytes of store data, so
	// CStoreWindows::PrepareForImport() will interpret the data as an empty path string.
	// Change to full ZeroMemory call when other store types are added!
	*((WCHAR*)&pItemDescriptor->StoreData) = L'\0';
}
