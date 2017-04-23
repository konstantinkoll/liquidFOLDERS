
#pragma once
#include "LF.h"


void SetFileContext(LFCoreAttributes* pCoreAttributes, BOOL Force=FALSE);
void SetNameExtFromFile(LFItemDescriptor* pItemDescriptor, LPCWSTR pFilename);
void SetFromFindData(LFCoreAttributes* pCoreAttributes, WIN32_FIND_DATA* pFindData);
void SetAttributesFromFile(LFItemDescriptor* pItemDescriptor, LPCWSTR pPath, BOOL Metadata=TRUE);
