
#pragma once
#include "LF.h"


void SetFileContext(LFCoreAttributes* pCoreAttributes, BOOL OnImport=TRUE);

BOOL SetAttributesFromAnnotation(LFItemDescriptor* pItemDescriptor, LPCWSTR pAnnotation);

void SetAttributesFromFindFileData(LFCoreAttributes* pCoreAttributes, WIN32_FIND_DATA& FindData);
void SetAttributesFromFindFileData(LFCoreAttributes* pCoreAttributes, LPCWSTR pPath);

void SetAttributesFromShell(LFItemDescriptor* pItemDescriptor, LPCWSTR pPath, BOOL OnImport=TRUE);
