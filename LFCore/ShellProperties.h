#pragma once
#include "LF.h"

static const GUID PropertyLF = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };

void SetFileContext(LFCoreAttributes* c, BOOL force=FALSE);
void SetAttributesFromFile(LFItemDescriptor* i, WCHAR* fn, BOOL metadata=TRUE);
void SetNameExtAddFromFile(LFItemDescriptor* i, WCHAR* fn);
