#pragma once
#include "LF.h"
#include "LFCore.h"

#define LFStoresHive     "Software\\liquidFOLDERS\\Stores"
#define MaxStores        100

#define RANDOMIZE() \
	SYSTEMTIME st; \
	GetSystemTime(&st); \
	srand(st.wMilliseconds*rand());

#define RAND_CHAR()      KeyChars[rand()%sizeof(KeyChars)]

extern CHAR DefaultStore[LFKeySize];
extern UINT StoreCount;

UINT MakeDefaultStore(LFStoreDescriptor* s);
void AppendGUID(LFStoreDescriptor* s, WCHAR* pPath, WCHAR* pSuffix=L"\\");
BOOL IsStoreMounted(LFStoreDescriptor* s);
void GetAutoPath(LFStoreDescriptor* s, WCHAR* pPath);
UINT DeleteStoreSettingsFromRegistry(LFStoreDescriptor* s);
void CreateNewStoreID(CHAR* StoreID);
void SetStoreAttributes(LFStoreDescriptor* s);
void InitStoreCache();
void AddStoresToSearchResult(LFSearchResult* sr);
LFStoreDescriptor* FindStore(CHAR* StoreID, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(GUID guid, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(WCHAR* DatPath, HANDLE* lock=NULL);
UINT FindStores(CHAR** IDs);
UINT UpdateStore(LFStoreDescriptor* s, BOOL UpdateTime=TRUE, BOOL MakeDefault=FALSE);
UINT DeleteStore(LFStoreDescriptor* s);
UINT MountVolume(CHAR cVolume, BOOL InternalCall=FALSE);
UINT UnmountVolume(CHAR cVolume, BOOL InternalCall=FALSE);
