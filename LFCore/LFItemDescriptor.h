#pragma once
#include "liquidFOLDERS.h"

#define ROUNDOFF           0.00000001

void AttributesToString(LFItemDescriptor* i);
size_t GetAttributeMaxCharacterCount(unsigned int attr);
size_t GetAttributeSize(unsigned int attr, const void* v);
void LFCore_API SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v, bool ToString=true, wchar_t* ustr=NULL);
