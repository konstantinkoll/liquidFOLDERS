#pragma once
#include "liquidFOLDERS.h"
#include "CIndex.h"
#include <shlobj.h>

DWORD CreateDir(LPWSTR lpPath);
void HideDir(LPWSTR lpPath);
void RemoveDir(LPWSTR lpPath);
bool DirFreeSpace(LPWSTR lpPathSrc, unsigned int Required);
unsigned int CopyDir(LPWSTR lpPathSrc, LPWSTR lpPathDst);
unsigned int ValidateStoreDirectories(LFStoreDescriptor* s);
void GetFileLocation(wchar_t* DatPath, LFCoreAttributes* ca, wchar_t* dst, size_t cCount);
bool FileExists(LPWSTR lpPath);
bool GetPIDLForStore(char* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
void SendLFNotifyMessage(unsigned int Msg, unsigned int Flags, HWND hWndSource);
void SendShellNotifyMessage(unsigned int Msg, char* StoreID=NULL, LPITEMIDLIST oldpidl=NULL, LPITEMIDLIST oldpidlDelegate=NULL);
void InitStores();
unsigned int OpenStore(LFStoreDescriptor* s, bool WriteAccess, CIndex* &Index1, CIndex* &Index2);
unsigned int OpenStore(char* key, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock);
