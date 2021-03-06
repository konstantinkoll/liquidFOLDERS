
#include "stdafx.h"
#include "Mutex.h"


HMUTEX hMutexStores;


// Mutex
//

void InitMutex()
{
	hMutexStores = CreateMutexA(NULL, FALSE, MUTEX_STORES);
}

BOOL GetMutex(HMUTEX hMutex)
{
	// Wait for max. 2 seconds
	DWORD dwWaitResult = WaitForSingleObject(hMutex, 2000);

	return ((dwWaitResult==WAIT_OBJECT_0) || (dwWaitResult==WAIT_ABANDONED));
}


BOOL GetMutexForStores()
{
	assert(hMutexStores);

	return GetMutex(hMutexStores);
}

void ReleaseMutexForStores()
{
	assert(hMutexStores);

	ReleaseMutex(hMutexStores);
}


BOOL GetMutexForStore(const LFStoreDescriptor* pStoreDescriptor, HMUTEX& hMutex)
{
	assert(pStoreDescriptor);

	CHAR ID[MAX_PATH];
	strcpy_s(ID, MAX_PATH, MUTEX_STORES);
	strcat_s(ID, MAX_PATH, "_");
	strcat_s(ID, MAX_PATH, pStoreDescriptor->StoreID);

	hMutex = CreateMutexA(NULL, FALSE, ID);
	if (hMutex==NULL)
		return FALSE;

	BOOL Result = GetMutex(hMutex);
	if (!Result)
		CloseHandle(hMutex);

	return Result;
}

void ReleaseMutexForStore(HANDLE hMutex)
{
	assert(hMutex);

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
}


// Files
//

UINT CreateFileConcurrent(LPCTSTR pPath, BOOL WriteAccess, DWORD dwCreationDisposition, HANDLE& hFile, BOOL Hidden)
{
#ifndef _DEBUG
	UINT Tries = 3;		// 3x ~500ms
#else
	UINT Tries = 2;		// 2x ~500ms
#endif

TryAgain:
	hFile = CreateFile(pPath, WriteAccess ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ, WriteAccess ? 0 : FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_FLAG_SEQUENTIAL_SCAN | (Hidden ? FILE_ATTRIBUTE_HIDDEN : FILE_ATTRIBUTE_NORMAL), NULL);
	if (hFile!=INVALID_HANDLE_VALUE)
		return FileOk;

	// Handle sharing violation
	if (GetLastError()==ERROR_SHARING_VIOLATION)
		if (Tries--)
		{
			Sleep(500+(rand() & 0x1F));		// 500ms + random

			goto TryAgain;
		}
		else
		{
			return FileSharingViolation;
		}

	// All other errors
	return FileNoAccess;
}
