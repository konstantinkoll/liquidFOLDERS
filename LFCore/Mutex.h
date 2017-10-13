
#pragma once
#include "LF.h"


typedef HANDLE HMUTEX;

#define MUTEX_STORES     "Global\\liquidFOLDERS"

#define FileOk                   0
#define FileSharingViolation     1
#define FileNoAccess             2


void InitMutex();
BOOL GetMutexForStores();
void ReleaseMutexForStores();
BOOL GetMutexForStore(const LFStoreDescriptor* pStoreDescriptor, HMUTEX& hMutex);
void ReleaseMutexForStore(HMUTEX hMutex);

UINT CreateFileConcurrent(LPCTSTR pPath, BOOL WriteAccess, DWORD dwCreationDisposition, HANDLE& hFile, BOOL Hidden=FALSE);

DWORD_PTR UpdateProgress(LFProgress* pProgress);
