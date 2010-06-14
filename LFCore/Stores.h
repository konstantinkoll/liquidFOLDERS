#pragma once
#include "liquidFOLDERS.h"
#include "CIndex.h"

DWORD CreateDir(LPCSTR lpPath);
void HideDir(LPCSTR lpPath);
void RemoveDir(LPCSTR lpPath);
bool DirFreeSpace(LPCSTR lpPathSrc, unsigned int Required);
unsigned int CopyDir(LPCSTR lpPathSrc, LPCSTR lpPathDst);
unsigned int ValidateStoreDirectories(LFStoreDescriptor* s);
void GetFileLocation(char* _Path, char* _FileID, char* _FileFormat, char* dst, size_t cCount);
void InitStores();
unsigned int OpenStore(LFStoreDescriptor* s, bool WriteAccess, CIndex* &Index1, CIndex* &Index2);
unsigned int OpenStore(char* key, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock);
