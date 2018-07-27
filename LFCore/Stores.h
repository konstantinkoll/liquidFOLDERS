
#pragma once
#include "CStore.h"
#include "Mutex.h"


#define RANDOMIZE() \
	SYSTEMTIME SystemTime; \
	GetSystemTime(&SystemTime); \
	srand(SystemTime.wMilliseconds*rand());

#define RAND_CHAR()     KeyChars[rand()%sizeof(KeyChars)]

inline BOOL SendLFNotifyMessage(UINT Msg)
{
	return SendNotifyMessage(HWND_BROADCAST, Msg, NULL, NULL);
}

UINT SaveStoreSettings(LFStoreDescriptor* pStoreDescriptor);

void CreateNewStoreID(ABSOLUTESTOREID& StoreID);
LFStoreDescriptor* FindStore(const ABSOLUTESTOREID& StoreID);
LFStoreDescriptor* FindStore(const GUID& UniqueID);
UINT UpdateStoreInCache(LFStoreDescriptor* pStoreDescriptor, BOOL UpdateFileTime=TRUE, BOOL MakeDefault=FALSE);

UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor);
void ChooseNewDefaultStore(BOOL OnInitialize=FALSE);

UINT StoreFlagsToType(const LFStoreDescriptor* pStoreDescriptor, UINT ItemType);
void GetDiskFreeSpaceForStore(LFStoreDescriptor& StoreDescriptor);
UINT GetStore(LFStoreDescriptor* pStoreDescriptor, CStore*& pStore);
UINT GetStore(const ABSOLUTESTOREID& StoreID, CStore*& pStore);
UINT OpenStore(const ABSOLUTESTOREID& StoreID, CStore*& pStore, BOOL WriteAccess=TRUE);
UINT OpenStore(const STOREID& StoreID, CStore*& pStore, BOOL WriteAccess=TRUE);

void QueryStores(LFSearchResult* pSearchResult);
void MountVolumes(UINT Mask, BOOL OnInitialize=FALSE);
void InitStores();

UINT MountVolume(CHAR cVolume, BOOL Mount=TRUE, BOOL OnInitialize=FALSE);
