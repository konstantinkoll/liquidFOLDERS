
#pragma once
#include "LF.h"


void SetFileContext(LFCoreAttributes& CoreAttributes, BOOL OnImport=TRUE);

BOOL SetAttributesFromAnnotation(LFItemDescriptor* pItemDescriptor, LPCWSTR pAnnotation);

void SetAttributesFromFindFileData(LFCoreAttributes& CoreAttributes, WIN32_FIND_DATA& FindData);
void SetAttributesFromFindFileData(LFCoreAttributes& CoreAttributes, LPCWSTR pPath);

void SetAttributesFromShell(LFItemDescriptor* pItemDescriptor, LPCWSTR pPath, BOOL OnImport=TRUE);
