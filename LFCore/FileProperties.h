
#pragma once
#include "LF.h"


void SetFileContext(LFCoreAttributes& CoreAttributes, BOOL OnImport=TRUE);

BOOL SetAttributesFromAnnotation(LFItemDescriptor* pItemDescriptor, LPCWSTR pAnnotation);

void SetAttributesFromFindData(LFCoreAttributes& CoreAttributes, const WIN32_FIND_DATA& FindData);
void SetAttributesFromFindData(LFCoreAttributes& CoreAttributes, LPCWSTR pPath);

void SetAttributesFromShell(LFItemDescriptor* pItemDescriptor, LPCWSTR pPath, BOOL OnImport=TRUE);
