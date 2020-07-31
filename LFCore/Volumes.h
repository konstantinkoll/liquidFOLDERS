
#pragma once
#include "LF.h"


void InitVolumes();
LPVOLUME GetVolume(CHAR cVolume, BOOL ForceAvailable=FALSE);
void UnmountVolume(CHAR cVolume);
