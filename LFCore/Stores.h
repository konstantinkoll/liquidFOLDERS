
#pragma once
#include "CStore.h"
#include "Mutex.h"


#define RANDOMIZE() \
	SYSTEMTIME SystemTime; \
	GetSystemTime(&SystemTime); \
	srand(SystemTime.wMilliseconds*rand());

#define RAND_CHAR()     KeyChars[rand()%sizeof(KeyChars)]

#define SendLFNotifyMessage(Message)     SendNotifyMessage(HWND_BROADCAST, Message, NULL, NULL);


void CreateNewStoreID(LPSTR pStoreID);
LFStoreDescriptor* FindStore(LPCSTR pStoreID);
LFStoreDescriptor* FindStore(const GUID& UniqueID);
LFStoreDescriptor* FindStore(LPCWSTR pDatPath);

UINT SaveStoreSettings(LFStoreDescriptor* pStoreDescriptor);
UINT UpdateStoreInCache(LFStoreDescriptor* pStoreDescriptor, BOOL UpdateFileTime=TRUE, BOOL MakeDefault=FALSE);

UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor);
void ChooseNewDefaultStore();

UINT StoreFlagsToType(const LFStoreDescriptor* pStoreDescriptor, UINT ItemType);
UINT GetStore(LFStoreDescriptor* pStoreDescriptor, CStore*& pStore);
UINT GetStore(LPCSTR pStoreID, CStore*& pStore);
UINT OpenStore(LPCSTR pStoreID, CStore*& pStore, BOOL WriteAccess=TRUE);

void QueryStores(LFSearchResult* pSearchResult);
void MountVolumes(UINT Mask, BOOL OnInitialize=FALSE);
void InitStores();

UINT MountVolume(CHAR cVolume, BOOL Mount=TRUE, BOOL OnInitialize=FALSE);
