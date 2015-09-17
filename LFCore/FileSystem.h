
#pragma once
#include "LF.h"


void SanitizeFileName(WCHAR* pDstName, SIZE_T cCount, WCHAR* pSrcName);
void AppendGUID(LFStoreDescriptor* pStoreDescriptor, WCHAR* pPath, WCHAR* pSuffix=L"\\");
void GetAutoPath(LFStoreDescriptor* pStoreDescriptor, WCHAR* pPath);
BOOL FileExists(LPWSTR lpPath);
BOOL RequiredSpaceAvailable(LPWSTR lpPath, UINT64 Required);
BOOL DirectoryWriteable(LPWSTR lpPath);
DWORD CreateDirectory(LPWSTR lpPath);
UINT CopyDirectory(LPWSTR lpPathSrc, LPWSTR lpPathDst);
BOOL DeleteDirectory(LPWSTR lpPath);
