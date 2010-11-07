#pragma once
#include "liquidFOLDERS.h"

#define LFStoresHive         "Software\\liquidFOLDERS\\Stores"
#define MaxStores            250

extern char DefaultStore[LFKeySize];
extern unsigned int StoreCount;

bool IsStoreMounted(LFStoreDescriptor* s);
void AppendGUID(LFStoreDescriptor* s, wchar_t* p);
void GetAutoPath(LFStoreDescriptor* s, wchar_t* p);
unsigned int ValidateStoreSettings(LFStoreDescriptor* s);
void InitStoreCache();
void CreateStoreKey(char* key);
void AddStoresToSearchResult(LFSearchResult* res, LFFilter* filter);
LFStoreDescriptor* FindStore(char* key, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(GUID guid, HANDLE* lock=NULL);
unsigned int FindStores(char** keys);
unsigned int UpdateStore(LFStoreDescriptor* s, bool MakeDefault=false);
unsigned int DeleteStore(LFStoreDescriptor* s);
