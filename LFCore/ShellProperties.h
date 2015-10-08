
#pragma once
#include "LF.h"


static const GUID PropertyLiquidFolders = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };


void SetFileContext(LFCoreAttributes* pCoreAttributes, BOOL Force=FALSE);
void SetNameExtFromFile(LFItemDescriptor* pItemDescriptor, WCHAR* pFilename);
void SetFromFindData(LFCoreAttributes* pCoreAttributes, WIN32_FIND_DATA* pFindData);
void SetAttributesFromFile(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, BOOL Metadata=TRUE);
