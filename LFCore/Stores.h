#pragma once
#include "liquidFOLDERS.h"

DWORD CreateDir(LPCSTR lpPath);
void RemoveDir(LPCSTR lpPath);
unsigned int ValidateStoreDirectories(LFStoreDescriptor* s);
void InitStores();
LFSearchResult* QueryStores(LFFilter* filter=NULL);
