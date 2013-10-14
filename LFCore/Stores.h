#pragma once
#include "liquidFOLDERS.h"
#include "CIndex.h"
#include <shlobj.h>

#define OPEN_STORE_PREFIX(StoreID, WriteAccess, ErrorOps, slot, idx1, idx2) \
	CIndex* idx1; \
	CIndex* idx2; \
	LFStoreDescriptor* slot; \
	HANDLE StoreLock = NULL; \
	unsigned int res = OpenStore(StoreID, WriteAccess, idx1, idx2, &slot, &StoreLock); \
	if (res!=LFOk) { ErrorOps; } else {

#define OPEN_STORE(StoreID, WriteAccess, ErrorOps) \
	OPEN_STORE_PREFIX(StoreID, WriteAccess, ErrorOps, slot, idx1, idx2);

#define CLOSE_STORE_PREFIX(idx1, idx2) \
	if (idx1) delete idx1; \
	if (idx2) delete idx2; \
	ReleaseMutexForStore(StoreLock); }

#define CLOSE_STORE() \
	CLOSE_STORE_PREFIX(idx1, idx2);

DWORD CreateDir(LPWSTR lpPath);
void HideDir(LPWSTR lpPath);
bool RemoveDir(LPWSTR lpPath);
bool DirFreeSpace(LPWSTR lpPath, unsigned int Required);
bool DirWriteable(LPWSTR lpPath);
unsigned int CopyDir(LPWSTR lpPathSrc, LPWSTR lpPathDst);
void SanitizeFileName(wchar_t* dst, size_t cCount, wchar_t* src);
unsigned int CreateStoreDirectories(LFStoreDescriptor* s);
void GetFileLocation(wchar_t* DatPath, LFCoreAttributes* ca, wchar_t* dst, size_t cCount);
bool FileExists(LPWSTR lpPath);
unsigned int PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, wchar_t* Dst, size_t cCount);
bool GetPIDLForStore(char* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
void SendLFNotifyMessage(unsigned int Msg, HWND hWndSource);
void SendShellNotifyMessage(unsigned int Msg, char* StoreID=NULL, LPITEMIDLIST oldpidl=NULL, LPITEMIDLIST oldpidlDelegate=NULL);
unsigned int RunMaintenance(LFStoreDescriptor* s, bool scheduled, LFProgress* pProgress=NULL);
unsigned int OpenStore(char* StoreID, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock);
