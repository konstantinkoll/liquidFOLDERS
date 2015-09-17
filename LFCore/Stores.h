
#pragma once
#include "CStore.h"
#include "Mutex.h"

void ChooseNewDefaultStore();
UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor);



#define RANDOMIZE() \
	SYSTEMTIME st; \
	GetSystemTime(&st); \
	srand(st.wMilliseconds*rand());

#define RAND_CHAR()     KeyChars[rand()%sizeof(KeyChars)]


void CreateNewStoreID(CHAR* pStoreID);
LFStoreDescriptor* FindStore(CHAR* pStoreID, HMUTEX* phMutex=NULL);
LFStoreDescriptor* FindStore(GUID UniqueID, HMUTEX* phMutex=NULL);
LFStoreDescriptor* FindStore(WCHAR* pDatPath, HMUTEX* phMutex=NULL);
UINT UpdateStoreInCache(LFStoreDescriptor* s, BOOL UpdateFileTime=TRUE, BOOL MakeDefault=FALSE);
UINT MountVolume(CHAR cVolume, BOOL OnInitialize=FALSE);






#define SendLFNotifyMessage(Message) SendMessage(HWND_BROADCAST, Message, NULL, NULL);

UINT GetStore(LFStoreDescriptor* pStoreDescriptor, CStore** ppStore, HMUTEX hMutex=NULL);
UINT GetStore(CHAR* StoreID, CStore** ppStore);
UINT OpenStore(CHAR* StoreID, BOOL WriteAccess, CStore** ppStore);



void QueryStores(LFSearchResult* pSearchResult);
void InitStores();

UINT UnmountVolume(CHAR cVolume);
