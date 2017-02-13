
#include "stdafx.h"
#include "Mutex.h"
#include <assert.h>


HMUTEX MutexStores;
extern LFMessageIDs LFMessages;


// Mutex
//

void InitMutex()
{
	MutexStores = CreateMutexA(NULL, FALSE, MUTEX_STORES);
}

BOOL GetMutex(HMUTEX hMutex)
{
	// Wait for max. 2 seconds
	DWORD dwWaitResult = WaitForSingleObject(hMutex, 2000);

	return ((dwWaitResult==WAIT_OBJECT_0) || (dwWaitResult==WAIT_ABANDONED));
}


BOOL GetMutexForStores()
{
	assert(MutexStores);

	return GetMutex(MutexStores);
}

void ReleaseMutexForStores()
{
	assert(MutexStores);

	ReleaseMutex(MutexStores);
}


BOOL GetMutexForStore(LFStoreDescriptor* pStoreDescriptor, HMUTEX* hMutex)
{
	assert(pStoreDescriptor);

	CHAR ID[MAX_PATH];
	strcpy_s(ID, MAX_PATH, MUTEX_STORES);
	strcat_s(ID, MAX_PATH, "_");
	strcat_s(ID, MAX_PATH, pStoreDescriptor->StoreID);

	*hMutex = CreateMutexA(NULL, FALSE, ID);
	if (*hMutex==NULL)
		return FALSE;

	BOOL Result = GetMutex(*hMutex);
	if (!Result)
		CloseHandle(*hMutex);

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

UINT CreateFileConcurrent(LPCTSTR lpFileName, BOOL WriteAccess, DWORD dwCreationDisposition, HANDLE& hFile, BOOL Hidden)
{
#ifndef _DEBUG
	UINT Tries = 3;		// 3x ~500ms
#else
	UINT Tries = 2;		// 2x ~500ms
#endif

TryAgain:
	hFile = CreateFile(lpFileName, WriteAccess ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ, WriteAccess ? 0 : FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_FLAG_SEQUENTIAL_SCAN | (Hidden ? FILE_ATTRIBUTE_HIDDEN : FILE_ATTRIBUTE_NORMAL), NULL);
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


// Calling UI threads
//

DWORD_PTR UpdateProgress(LFProgress* pProgress)
{
	DWORD_PTR Result;

	if (SendMessageTimeout(pProgress->hWnd, LFMessages.UpdateProgress, (WPARAM)pProgress, NULL, SMTO_ABORTIFHUNG, 5000, &Result))
		return Result;

	return TRUE;
}
