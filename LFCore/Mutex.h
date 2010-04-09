#pragma once
#include "liquidFOLDERS.h"

void InitMutex();
bool GetMutex(HANDLE m);
HANDLE GetMutexForStore(LFStoreDescriptor* s);
void ReleaseMutexForStore(HANDLE m);
