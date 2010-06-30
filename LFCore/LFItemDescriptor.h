#pragma once
#include "liquidFOLDERS.h"

size_t GetAttributeMaxCharacterCount(unsigned int attr);
size_t GetAttributeSize(unsigned int attr, const void* v);
void LFCore_API SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v);
LFItemDescriptor* AllocFolderDescriptor(const wchar_t* Name, const wchar_t* Comment, const wchar_t* Description, const char* StoreID, const char* FileID, __int64* Size, unsigned int IconID, unsigned int CategoryID, LFFilter* Filter);
