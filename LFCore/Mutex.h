
#pragma once
#include "LF.h"


typedef HANDLE HMUTEX;

#define MUTEX_STORES     "Global\\liquidFOLDERS"


void InitMutex();
BOOL GetMutexForStores();
void ReleaseMutexForStores();
BOOL GetMutexForStore(LFStoreDescriptor* pStoreDescriptor, HMUTEX* hMutex);
void ReleaseMutexForStore(HMUTEX hMutex);
DWORD_PTR UpdateProgress(LFProgress* pProgress);
