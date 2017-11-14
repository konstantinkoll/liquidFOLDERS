
#include "stdafx.h"
#include "FileSystem.h"
#include "LFCore.h"
#include <assert.h>
#include <shlobj.h>
#include <winioctl.h>


void SanitizeFileName(LPWSTR lpDstName, SIZE_T cCount, LPCWSTR lpSrcName)
{
	assert(lpDstName);
	assert(lpSrcName);

	wcscpy_s(lpDstName, cCount, lpSrcName);

	while (*lpDstName)
	{
		if ((*lpDstName<L' ') || (wcschr(L"<>:\"/\\|?*", *lpDstName)))
			*lpDstName = L'_';

		lpDstName++;
	}
}

void AppendGUID(LPWSTR lpPath, LFStoreDescriptor* pStoreDescriptor, LPCWSTR pSuffix)
{
	assert(lpPath);
	assert(pStoreDescriptor);
	assert(pSuffix);

	WCHAR szGUID[MAX_PATH];
	if (StringFromGUID2(pStoreDescriptor->UniqueID, szGUID, MAX_PATH))
	{
		wcscat_s(lpPath, MAX_PATH, szGUID);
		wcscat_s(lpPath, MAX_PATH, pSuffix);
	}
}

void GetAutoPath(LFStoreDescriptor* pStoreDescriptor, LPWSTR lpPath)
{
	assert(pStoreDescriptor);
	assert(lpPath);

	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"Stores", lpPath);
	wcscat_s(lpPath, MAX_PATH, L"\\");
	AppendGUID(lpPath, pStoreDescriptor);
}

BOOL FileExists(LPCWSTR lpPath, WIN32_FIND_DATA* pFindData)
{
	assert(lpPath);

	WIN32_FIND_DATA FindFileData;

	if (!pFindData)
		pFindData = &FindFileData;

	HANDLE hFind = FindFirstFile(lpPath, pFindData);

	BOOL Result = (hFind!=INVALID_HANDLE_VALUE);
	if (Result)
		FindClose(hFind);

	return Result;
}

BOOL DirectoryExists(LPCWSTR lpPath)
{
	assert(lpPath);

	WIN32_FIND_DATA FindFileData;
	if (FileExists(lpPath, &FindFileData))
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return TRUE;

	return FALSE;
}

void CompressFile(HANDLE hFile, CHAR cVolume)
{
	BY_HANDLE_FILE_INFORMATION FileInformation;
	if (GetFileInformationByHandle(hFile, &FileInformation))
		if ((FileInformation.dwFileAttributes & (FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_COMPRESSED))==0)
		{
			WCHAR Root[4] = L" :\\";
			Root[0] = cVolume;

			DWORD Flags;
			if (GetVolumeInformation(Root, NULL, 0, NULL, NULL, &Flags, NULL, 0))
				if (Flags & FS_FILE_COMPRESSION)
				{
					USHORT Mode = COMPRESSION_FORMAT_LZNT1;
					DWORD Returned;

					DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &Mode, sizeof(Mode), NULL, 0, &Returned, NULL);
				}
		}
}

void HideFile(LPCWSTR lpPath)
{
	assert(lpPath);

	DWORD dwFileAttributes = GetFileAttributes(lpPath);

	if (dwFileAttributes!=INVALID_FILE_ATTRIBUTES)
		SetFileAttributes(lpPath, dwFileAttributes | FILE_ATTRIBUTE_HIDDEN);
}

BOOL RequiredSpaceAvailable(LPCWSTR lpPath, UINT64 Required)
{
	assert(lpPath);

	ULARGE_INTEGER FreeBytesAvailable;

	return GetDiskFreeSpaceEx(lpPath, &FreeBytesAvailable, NULL, NULL) ? FreeBytesAvailable.QuadPart>=Required : FALSE;
}

BOOL VolumeWriteable(CHAR cVolume)
{
	WCHAR Path[] = L" :\\";
	Path[0] = cVolume;

	DWORD dwFlags;
	if (GetVolumeInformation(Path, NULL, 0, NULL, 0, &dwFlags, NULL, 0))
		if ((dwFlags & FILE_READ_ONLY_VOLUME)==0)
			return TRUE;

	return FALSE;
}

DWORD CreateDirectory(LPCWSTR lpPath)
{
	assert(lpPath);

	return CreateDirectory(lpPath, NULL) ? ERROR_SUCCESS : GetLastError();
}

UINT CopyDirectory(LPCWSTR lpPathSrc, LPCWSTR lpPathDst)
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

BOOL DeleteDirectory(LPCWSTR lpPath)
{
	BOOL Result = TRUE;

	// Dateien löschen
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

	// Verzeichnis löschen
	if (Result)
		Result = RemoveDirectory(lpPath);

	return Result;
}
