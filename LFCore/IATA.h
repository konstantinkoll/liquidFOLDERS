#pragma once
#include "LF.h"

void InitAirportDatabase();
bool GetItemCoordinates(LFItemDescriptor* i, unsigned int PreferredAttr, LFGeoCoordinates* coord);
void CustomizeFolderForAirport(LFItemDescriptor* i, LFAirport* airport);
