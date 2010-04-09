#pragma once
#include "liquidFOLDERS.h"

#define LFStoresHive         "Software\\liquidFOLDERS\\Stores"
#define MaxStores            250

extern char DefaultStore[LFKeySize];
extern unsigned int StoreCount;

bool IsStoreMounted(LFStoreDescriptor* s);
void GetAutoPath(LFStoreDescriptor* s, char* p);
unsigned int ValidateStoreSettings(LFStoreDescriptor* s);
void InitStoreCache();
void CreateStoreKey(char* key);
LFItemDescriptor* StoreDescriptor2ItemDescriptor(LFStoreDescriptor* s);
void AddStores(LFSearchResult* res);
LFStoreDescriptor* FindStore(char* key, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(_GUID guid, HANDLE* lock=NULL);
unsigned int UpdateStore(LFStoreDescriptor* s, BOOL MakeDefault=FALSE);
unsigned int DeleteStore(LFStoreDescriptor* s);
