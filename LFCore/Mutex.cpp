
#include "stdafx.h"
#include "Mutex.h"
#include <assert.h>


HMUTEX MutexStores;


void InitMutex()
{
	MutexStores = CreateMutexA(NULL, FALSE, MUTEX_STORES);
}

BOOL GetMutex(HMUTEX hMutex)
{
	// Wait for max. 10 seconds
	DWORD dwWaitResult = WaitForSingleObject(hMutex, 10000);

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
}
