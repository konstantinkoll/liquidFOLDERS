
#pragma once
#include "LF.h"


void SanitizeFileName(WCHAR* pDstName, SIZE_T cCount, WCHAR* pSrcName);
void AppendGUID(WCHAR* pPath, LFStoreDescriptor* pStoreDescriptor, WCHAR* pSuffix=L"\\");
void GetAutoPath(LFStoreDescriptor* pStoreDescriptor, WCHAR* pPath);
BOOL FileExists(LPWSTR lpPath, WIN32_FIND_DATA* pFindData=NULL);
BOOL DirectoryExists(LPWSTR lpPath);
void CompressFile(HANDLE hFile, WCHAR cDrive);
void HideFile(WCHAR* pPath);
BOOL RequiredSpaceAvailable(LPWSTR lpPath, UINT64 Required);
BOOL DirectoryWriteable(LPWSTR lpPath);
DWORD CreateDirectory(LPWSTR lpPath);
UINT CopyDirectory(LPWSTR lpPathSrc, LPWSTR lpPathDst);
BOOL DeleteDirectory(LPWSTR lpPath);
