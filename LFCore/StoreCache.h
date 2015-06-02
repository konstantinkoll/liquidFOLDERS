#pragma once
#include "liquidFOLDERS.h"

#define LFStoresHive     "Software\\liquidFOLDERS\\Stores"
#define MaxStores        250

#define RANDOMIZE() \
	SYSTEMTIME st; \
	GetSystemTime(&st); \
	srand(st.wMilliseconds*rand());

#define RAND_CHAR()      KeyChars[rand()%sizeof(KeyChars)]

extern char DefaultStore[LFKeySize];
extern unsigned int StoreCount;

unsigned int MakeDefaultStore(LFStoreDescriptor* s);
void AppendGUID(LFStoreDescriptor* s, wchar_t* pPath, wchar_t* pSuffix=L"\\");
bool IsStoreMounted(LFStoreDescriptor* s);
void GetAutoPath(LFStoreDescriptor* s, wchar_t* pPath);
unsigned int DeleteStoreSettingsFromRegistry(LFStoreDescriptor* s);
void CreateNewStoreID(char* StoreID);
void SetStoreAttributes(LFStoreDescriptor* s);
void InitStoreCache();
void AddStoresToSearchResult(LFSearchResult* sr);
LFStoreDescriptor* FindStore(char* StoreID, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(GUID guid, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(wchar_t* DatPath, HANDLE* lock=NULL);
unsigned int FindStores(char** IDs);
unsigned int UpdateStore(LFStoreDescriptor* s, bool UpdateTime=true, bool MakeDefault=false);
unsigned int DeleteStore(LFStoreDescriptor* s);
unsigned int MountVolume(char cVolume, bool InternalCall=false);
unsigned int UnmountVolume(char cVolume, bool InternalCall=false);
