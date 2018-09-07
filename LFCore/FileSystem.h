
#pragma once
#include "LF.h"


void SanitizeFileName(LPWSTR lpDstName, SIZE_T cCount, LPCWSTR lpSrcName);
void AppendGUID(LPWSTR lpPath, const LFStoreDescriptor& StoreDescriptor, LPCWSTR pSuffix=L"\\");
void GetAutoPath(const LFStoreDescriptor& StoreDescriptor, LPWSTR lpPath);
BOOL FileExists(LPCWSTR lpPath, WIN32_FIND_DATA* pFindData=NULL);
BOOL DirectoryExists(LPCWSTR lpPath);
void CompressFile(HANDLE hFile, CHAR cVolume);
void HideFile(LPCWSTR lpPath);
BOOL RequiredSpaceAvailable(LPCWSTR lpPath, UINT64 Required);
BOOL VolumeWriteable(CHAR cVolume);
DWORD CreateDirectory(LPCWSTR lpPath);
UINT CopyDirectory(LPCWSTR lpPathSrc, LPCWSTR lpPathDst);
BOOL DeleteDirectory(LPCWSTR lpPath);
