#pragma once
#include "liquidFOLDERS.h"

void InitMutex();
bool GetMutex(HANDLE m);
bool GetMutexForStore(LFStoreDescriptor* s, HANDLE* m);
void ReleaseMutexForStore(HANDLE m);
