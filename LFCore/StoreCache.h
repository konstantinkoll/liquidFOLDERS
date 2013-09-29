#pragma once
#include "liquidFOLDERS.h"

#define LFStoresHive         "Software\\liquidFOLDERS\\Stores"
#define MaxStores            250

extern char DefaultStore[LFKeySize];
extern unsigned int StoreCount;

void AppendGUID(LFStoreDescriptor* s, wchar_t* p);
void GetAutoPath(LFStoreDescriptor* s, wchar_t* p);
unsigned int DeleteStoreSettingsFromRegistry(LFStoreDescriptor* s);
unsigned int ValidateStoreSettings(LFStoreDescriptor* s, int Source=-1);
void InitStoreCache();
void CreateStoreKey(char* key);
void AddStoresToSearchResult(LFSearchResult* res, LFFilter* filter);
LFStoreDescriptor* FindStore(char* key, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(GUID guid, HANDLE* lock=NULL);
LFStoreDescriptor* FindStore(wchar_t* datpath, HANDLE* lock=NULL);
unsigned int FindStores(char** keys);
unsigned int UpdateStore(LFStoreDescriptor* s, bool UpdateTime=true, bool MakeDefault=false);
unsigned int DeleteStore(LFStoreDescriptor* s);
unsigned int MountDrive(char d, bool InternalCall=false);
unsigned int UnmountDrive(char d, bool InternalCall=false);
