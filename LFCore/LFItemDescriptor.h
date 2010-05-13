#pragma once
#include "liquidFOLDERS.h"

#define ROUNDOFF           0.00000001

void AttributesToString(LFItemDescriptor* i);
size_t GetAttributeMaxCharacterCount(unsigned int attr);
size_t GetAttributeSize(unsigned int attr, const void* v);
void LFCore_API SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v, bool ToString=true, wchar_t* ustr=NULL);
LFItemDescriptor* AllocFolderDescriptor(const wchar_t* Name, const wchar_t* Comment, const wchar_t* Hint, const char* StoreID, const char* FileID, unsigned int IconID, unsigned int CategoryID, LFFilter* Filter);
