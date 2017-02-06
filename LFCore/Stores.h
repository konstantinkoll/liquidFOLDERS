
#pragma once
#include "CStore.h"
#include "Mutex.h"


#define RANDOMIZE() \
	SYSTEMTIME SystemTime; \
	GetSystemTime(&SystemTime); \
	srand(SystemTime.wMilliseconds*rand());

#define RAND_CHAR()     KeyChars[rand()%sizeof(KeyChars)]

#define SendLFNotifyMessage(Message)     PostMessage(HWND_BROADCAST, Message, NULL, NULL);


void CreateNewStoreID(CHAR* pStoreID);
LFStoreDescriptor* FindStore(const CHAR* pStoreID, HMUTEX* phMutex=NULL);
LFStoreDescriptor* FindStore(const GUID UniqueID, HMUTEX* phMutex=NULL);
LFStoreDescriptor* FindStore(const WCHAR* pDatPath, HMUTEX* phMutex=NULL);

UINT SaveStoreSettings(LFStoreDescriptor* pStoreDescriptor);
UINT UpdateStoreInCache(LFStoreDescriptor* pStoreDescriptor, BOOL UpdateFileTime=TRUE, BOOL MakeDefault=FALSE);

UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor);
void ChooseNewDefaultStore();

UINT GetStore(LFStoreDescriptor* pStoreDescriptor, CStore** ppStore, HMUTEX hMutex=NULL);
UINT GetStore(const CHAR* pStoreID, CStore** ppStore);
UINT OpenStore(const CHAR* pStoreID, BOOL WriteAccess, CStore** ppStore);

void QueryStores(LFSearchResult* pSearchResult);
void MountVolumes(UINT Mask, BOOL OnInitialize=FALSE);
void InitStores();

UINT MountVolume(CHAR cVolume, BOOL Mount=TRUE, BOOL OnInitialize=FALSE);
