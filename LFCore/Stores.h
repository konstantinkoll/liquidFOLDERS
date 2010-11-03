#pragma once
#include "liquidFOLDERS.h"
#include "CIndex.h"
#include <shlobj.h>

DWORD CreateDir(LPCSTR lpPath);
void HideDir(LPCSTR lpPath);
void RemoveDir(LPCSTR lpPath);
bool DirFreeSpace(LPCSTR lpPathSrc, unsigned int Required);
unsigned int CopyDir(LPCSTR lpPathSrc, LPCSTR lpPathDst);
unsigned int ValidateStoreDirectories(LFStoreDescriptor* s);
void GetFileLocation(char* _Path, char* _FileID, char* _FileFormat, char* dst, size_t cCount);
bool FileExists(char* path);
bool GetPIDLForStore(char* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
void SendLFNotifyMessage(unsigned int Msg, unsigned int Flags, HWND hWndSource);
void SendShellNotifyMessage(unsigned int Msg, char* StoreID=NULL, LPITEMIDLIST oldpidl=NULL, LPITEMIDLIST oldpidlDelegate=NULL);
void InitStores();
unsigned int OpenStore(LFStoreDescriptor* s, bool WriteAccess, CIndex* &Index1, CIndex* &Index2);
unsigned int OpenStore(char* key, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock);
