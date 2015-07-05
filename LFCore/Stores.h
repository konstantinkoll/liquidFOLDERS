#pragma once
#include "LF.h"
#include "CIndex.h"
#include <shlobj.h>

#define OPEN_STORE_PREFIX(StoreID, WriteAccess, ErrorOps, slot, idx1, idx2) \
	CIndex* idx1; \
	CIndex* idx2; \
	LFStoreDescriptor* slot; \
	HANDLE StoreLock = NULL; \
	UINT Result = OpenStore(StoreID, WriteAccess, idx1, idx2, &slot, &StoreLock); \
	if (Result!=LFOk) { ErrorOps; } else {

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
BOOL RemoveDir(LPWSTR lpPath);
BOOL DirFreeSpace(LPWSTR lpPath, UINT Required);
BOOL DirWriteable(LPWSTR lpPath);
UINT CopyDir(LPWSTR lpPathSrc, LPWSTR lpPathDst);
void SanitizeFileName(WCHAR* dst, SIZE_T cCount, WCHAR* src);
UINT CreateStoreDirectories(LFStoreDescriptor* s);
void GetFileLocation(WCHAR* DatPath, LFCoreAttributes* ca, WCHAR* dst, SIZE_T cCount);
BOOL FileExists(LPWSTR lpPath);
UINT PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, WCHAR* Dst, SIZE_T cCount);
BOOL GetPIDLForStore(CHAR* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate);
void SendLFNotifyMessage(UINT Msg);
void SendShellNotifyMessage(UINT Msg, CHAR* StoreID=NULL, LPITEMIDLIST oldpidl=NULL, LPITEMIDLIST oldpidlDelegate=NULL);
UINT RunMaintenance(LFStoreDescriptor* s, BOOL scheduled, LFProgress* pProgress=NULL);
UINT OpenStore(CHAR* StoreID, BOOL WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock);
