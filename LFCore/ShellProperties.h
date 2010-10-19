#pragma once
#include "liquidFOLDERS.h"

static const GUID PropertyLF =
	{ 0x3f2d914f, 0xfe57, 0x414f, { 0x9f, 0x88, 0xa3, 0x77, 0xc7, 0x84, 0x1d, 0xa4 } };

void SetFileDomainAndSlave(LFItemDescriptor* i);
void SetAttributesFromFile(LFItemDescriptor* i, wchar_t* fn);
void SetNameExtAddFromFile(LFItemDescriptor* i, wchar_t* fn);
