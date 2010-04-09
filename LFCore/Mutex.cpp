#include "StdAfx.h"
#include "Mutex.h"


HANDLE Mutex_Stores;


HANDLE CreateMutex(LPCSTR lpName)
{
	HANDLE m = CreateMutexA(NULL, FALSE, lpName);
	if (!m)
		m = OpenMutexA(NULL, FALSE, lpName);

	return m;
}

void InitMutex()
{
	Mutex_Stores = CreateMutex("liquidFOLDERS_Mutex_Stores");
}

bool GetMutex(HANDLE m)
{
	// Wait for max. 20 seconds
	DWORD dwWaitResult = WaitForSingleObject(m, 20000);

	return ((dwWaitResult==WAIT_OBJECT_0) || (dwWaitResult==WAIT_ABANDONED));
}

HANDLE GetMutexForStore(LFStoreDescriptor* s)
{
	char ID[MAX_PATH];
	strcpy_s(ID, MAX_PATH, "liquidFOLDERS_Mutex_Store_");
	strcat_s(ID, MAX_PATH, s->StoreID);

	return CreateMutexA(NULL, TRUE, ID);
}

void ReleaseMutexForStore(HANDLE m)
{
	ReleaseMutex(m);
	CloseHandle(m);
}
