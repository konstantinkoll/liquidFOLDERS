
#include "stdafx.h"
#include "FileSystem.h"
#include "LF.h"
#include <assert.h>
#include <shlobj.h>


void SanitizeFileName(WCHAR* pDstName, SIZE_T cCount, WCHAR* pSrcName)
{
	assert(pDstName);
	assert(pSrcName);

	wcscpy_s(pDstName, cCount, pSrcName);

	while (*pDstName)
	{
		if ((*pDstName<L' ') || (wcschr(L"<>:\"/\\|?*", *pDstName)))
			*pDstName = L'_';

		pDstName++;
	}
}

void AppendGUID(LFStoreDescriptor* pStoreDescriptor, WCHAR* pPath, WCHAR* pSuffix)
{
	assert(pStoreDescriptor);
	assert(pPath);
	assert(pSuffix);

	WCHAR szGUID[MAX_PATH];
	if (StringFromGUID2(pStoreDescriptor->UniqueID, szGUID, MAX_PATH))
	{
		wcscat_s(pPath, MAX_PATH, szGUID);
		wcscat_s(pPath, MAX_PATH, pSuffix);
	}
}

void GetAutoPath(LFStoreDescriptor* pStoreDescriptor, WCHAR* pPath)
{
	assert(pStoreDescriptor);
	assert(pPath);

	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"Stores", pPath);
	wcscat_s(pPath, MAX_PATH, L"\\");
	AppendGUID(pStoreDescriptor, pPath);
}

BOOL FileExists(LPWSTR lpPath, WIN32_FIND_DATA* pFindData)
{
	WIN32_FIND_DATA FindFileData;

	if (!pFindData)
		pFindData = &FindFileData;

	HANDLE hFind = FindFirstFile(lpPath, pFindData);

	BOOL Result = (hFind!=INVALID_HANDLE_VALUE);
	if (Result)
		FindClose(hFind);

	return Result;
}

BOOL RequiredSpaceAvailable(LPWSTR lpPath, UINT64 Required)
{
	ULARGE_INTEGER FreeBytesAvailable;

	return GetDiskFreeSpaceEx(lpPath, &FreeBytesAvailable, NULL, NULL) ? FreeBytesAvailable.QuadPart>=Required : FALSE;
}

BOOL DirectoryWriteable(LPWSTR lpPath)
{
	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, lpPath);
	wcscat_s(Path, MAX_PATH, L"LF_TEST.BIN");

	HANDLE hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;

	CloseHandle(hFile);

	return DeleteFile(Path);
}

DWORD CreateDirectory(LPWSTR lpPath)
{
	return CreateDirectory(lpPath, NULL) ? ERROR_SUCCESS : GetLastError();
}

UINT CopyDirectory(LPWSTR lpPathSrc, LPWSTR lpPathDst)
{
	UINT Result = LFOk;

	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, lpPathSrc);
	wcscat_s(Path, MAX_PATH, L"*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(Path, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
			{
				WCHAR PathSource[MAX_PATH];
				wcscpy_s(PathSource, MAX_PATH, lpPathSrc);
				wcscat_s(PathSource, MAX_PATH, FindFileData.cFileName);

				WCHAR PathDestination[MAX_PATH];
				wcscpy_s(PathDestination, MAX_PATH, lpPathDst);
				wcscat_s(PathDestination, MAX_PATH, FindFileData.cFileName);

				if (!CopyFile(PathSource, PathDestination, FALSE))
				{
					Result = LFCannotCopyIndex;
					break;
				}
			}
		}
		while (FindNextFile(hFind, &FindFileData)!=0);

		FindClose(hFind);
	}

	return Result;
}

BOOL DeleteDirectory(LPWSTR lpPath)
{
	BOOL Result = TRUE;

	// Dateien l�schen
	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, lpPath);
	wcscat_s(Path, MAX_PATH, L"*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(Path, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
			{
				WCHAR FileName[MAX_PATH];
				wcscpy_s(FileName, MAX_PATH, lpPath);
				wcscat_s(FileName, MAX_PATH, FindFileData.cFileName);

				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if ((wcscmp(FindFileData.cFileName, L".")!=0) && (wcscmp(FindFileData.cFileName, L"..")!=0))
					{
						wcscat_s(FileName, MAX_PATH, L"\\");
						if (!DeleteDirectory(FileName))
							Result = FALSE;
					}
				}
				else
				{
					if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
						SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);

					if (!DeleteFile(FileName))
						Result = FALSE;
				}
			}
		}
		while (FindNextFile(hFind, &FindFileData)!=0);

		FindClose(hFind);
	}

	// Verzeichnis l�schen
	if (Result)
		Result = RemoveDirectory(lpPath);

	return Result;
}