
#include "stdafx.h"
#include "Mutex.h"
#include <assert.h>


HANDLE Mutex_Stores;


void InitMutex()
{
	Mutex_Stores = CreateMutexA(NULL, FALSE, LFCM_Stores);
}

bool GetMutex(HANDLE m)
{
	// Wait for max. 20 seconds
	DWORD dwWaitResult = WaitForSingleObject(m, 20000);

	return ((dwWaitResult==WAIT_OBJECT_0) || (dwWaitResult==WAIT_ABANDONED));
}

bool GetMutexForStore(LFStoreDescriptor* s, HANDLE* m)
{
	assert(s);

	char ID[MAX_PATH];
	strcpy_s(ID, MAX_PATH, LFCM_Store);
	strcat_s(ID, MAX_PATH, s->StoreID);

	*m = CreateMutexA(NULL, FALSE, ID);

	bool Result = GetMutex(*m);
	if (!Result)
	{
		CloseHandle(*m);
		*m = NULL;
	}

	return Result;
}

void ReleaseMutexForStore(HANDLE m)
{
	ReleaseMutex(m);
	CloseHandle(m);
}
