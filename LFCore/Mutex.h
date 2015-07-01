#pragma once
#include "LF.h"

void InitMutex();
BOOL GetMutex(HANDLE m);
BOOL GetMutexForStore(LFStoreDescriptor* s, HANDLE* m);
void ReleaseMutexForStore(HANDLE m);
