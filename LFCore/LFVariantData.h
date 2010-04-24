#pragma once
#include "liquidFOLDERS.h"

#define ROUNDOFF           0.00000001

void FreeAttribute(LFItemDescriptor* f, unsigned int attr);
void SetAttributeUnicodeString(LFItemDescriptor* f, unsigned int attr, const wchar_t* str);
void SetAttributeAnsiString(LFItemDescriptor* f, unsigned int attr, const char* str, wchar_t* ustr=NULL);
void SetAttributeFourCC(LFItemDescriptor* f, unsigned int attr, unsigned int c, wchar_t* ustr=NULL);
void SetAttributeUINT(LFItemDescriptor* f, unsigned int attr, unsigned int v, wchar_t* ustr=NULL);
void SetAttributeINT64(LFItemDescriptor* f, unsigned int attr, __int64 v, wchar_t* ustr=NULL);
void SetAttributeFraction(LFItemDescriptor* f, unsigned int attr, const LFFraction frac, wchar_t* ustr=NULL);
void SetAttributeDouble(LFItemDescriptor* f, unsigned int attr, double d, wchar_t* ustr=NULL);
void SetAttributeRating(LFItemDescriptor* f, unsigned int attr, unsigned char r);
void SetAttributeFlags(LFItemDescriptor* f, unsigned int attr, unsigned int v);
void SetAttributeGeoCoordinates(LFItemDescriptor* f, unsigned int attr, const LFGeoCoordinates c, wchar_t* ustr=NULL);
void SetAttributeTime(LFItemDescriptor* f, unsigned int attr, const FILETIME t, wchar_t* ustr=NULL);
void SetAttributeDuration(LFItemDescriptor* f, unsigned int attr, unsigned int d, wchar_t* ustr=NULL);
