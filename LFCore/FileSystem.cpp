
#include "stdafx.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "Volumes.h"
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

void AppendGUID(LPWSTR lpPath, const LFStoreDescriptor& StoreDescriptor, LPCWSTR pSuffix)
{
	assert(lpPath);
	assert(pSuffix);

	WCHAR szGUID[MAX_PATH];
	if (StringFromGUID2(StoreDescriptor.UniqueID, szGUID, MAX_PATH))
	{
		wcscat_s(lpPath, MAX_PATH, szGUID);
		wcscat_s(lpPath, MAX_PATH, pSuffix);
	}
}

void GetAutoPath(const LFStoreDescriptor& StoreDescriptor, LPWSTR lpPath)
{
	assert(lpPath);

	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"Stores", lpPath);
	wcscat_s(lpPath, MAX_PATH, L"\\");
	AppendGUID(lpPath, StoreDescriptor);
}

BOOL FileExists(LPCWSTR lpPath, WIN32_FIND_DATA& FindData)
{
	assert(lpPath);

	HANDLE hFind = FindFirstFile(lpPath, &FindData);

	BOOL Result = (hFind!=INVALID_HANDLE_VALUE);
	if (Result)
		FindClose(hFind);

	return Result;
}

BOOL DirectoryExists(LPCWSTR lpPath)
{
	assert(lpPath);

	WIN32_FIND_DATA FindData;
	return FileExists(lpPath, FindData) && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

void CompressFile(HANDLE hFile, CHAR cVolume)
{
	BY_HANDLE_FILE_INFORMATION FileInformation;
	if (GetFileInformationByHandle(hFile, &FileInformation) && ((FileInformation.dwFileAttributes & (FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_COMPRESSED))==0))
		if (GetVolume(cVolume)->Flags & LFFlagsCompressionAllowed)
		{
			USHORT Mode = COMPRESSION_FORMAT_LZNT1;
			DWORD Returned;

			DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, &Mode, sizeof(Mode), NULL, 0, &Returned, NULL);
		}
}

BOOL CompressFile(LPCWSTR lpPath)
{
	assert(lpPath);

	BOOL Result = FALSE;

	HANDLE hFile;
	if ((hFile=CreateFile(lpPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL))!=INVALID_HANDLE_VALUE)
	{
		// Handle extended path types
		const WCHAR* pVolume = lpPath;
		while (*pVolume && (*pVolume<L'A' || *pVolume>L'Z'))
			pVolume++;

		if (*pVolume)
		{
			CompressFile(hFile, (CHAR)*pVolume);

			BY_HANDLE_FILE_INFORMATION FileInformation;
			if (GetFileInformationByHandle(hFile, &FileInformation) && (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED))
				Result = TRUE;
		}

		CloseHandle(hFile);
	}

	return Result;
}

void HideFile(LPCWSTR lpPath)
{
	assert(lpPath);

	const DWORD dwFileAttributes = GetFileAttributes(lpPath);

	if (dwFileAttributes!=INVALID_FILE_ATTRIBUTES)
		SetFileAttributes(lpPath, dwFileAttributes | FILE_ATTRIBUTE_HIDDEN);
}

BOOL RequiredSpaceAvailable(LPCWSTR lpPath, UINT64 Required)
{
	assert(lpPath);

	ULARGE_INTEGER FreeBytesAvailable;

	return GetDiskFreeSpaceEx(lpPath, &FreeBytesAvailable, NULL, NULL) ? FreeBytesAvailable.QuadPart>=Required : FALSE;
}

DWORD CreateDirectory(LPCWSTR lpPath)
{
	assert(lpPath);

	return CreateDirectory(lpPath, NULL) ? ERROR_SUCCESS : GetLastError();
}

UINT CopyDirectory(LPCWSTR lpPathSrc, LPCWSTR lpPathDst, BOOL Compress)
{
	UINT Result = LFOk;

	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, lpPathSrc);
	wcscat_s(Path, MAX_PATH, L"*");

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(Path, &FindData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
			{
				// Copy file
				WCHAR PathSource[MAX_PATH];
				wcscpy_s(PathSource, MAX_PATH, lpPathSrc);
				wcscat_s(PathSource, MAX_PATH, FindData.cFileName);

				WCHAR PathDestination[MAX_PATH];
				wcscpy_s(PathDestination, MAX_PATH, lpPathDst);
				wcscat_s(PathDestination, MAX_PATH, FindData.cFileName);

				if (!CopyFile(PathSource, PathDestination, FALSE))
				{
					Result = LFCannotCopyIndex;
					break;
				}

				// Compress destination file
				if (Compress)
					CompressFile(PathDestination);
			}
		}
		while (FindNextFile(hFind, &FindData)!=0);

		FindClose(hFind);
	}

	return Result;
}

BOOL DeleteDirectory(LPCWSTR lpPath)
{
	BOOL Result = TRUE;

	// Remove files
	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, lpPath);
	wcscat_s(Path, MAX_PATH, L"*");

	WIN32_FIND_DATA FindData;
	HANDLE hFind = FindFirstFile(Path, &FindData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
			{
				WCHAR FileName[MAX_PATH];
				wcscpy_s(FileName, MAX_PATH, lpPath);
				wcscat_s(FileName, MAX_PATH, FindData.cFileName);

				if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if ((wcscmp(FindData.cFileName, L".")!=0) && (wcscmp(FindData.cFileName, L"..")!=0))
					{
						wcscat_s(FileName, MAX_PATH, L"\\");
						if (!DeleteDirectory(FileName))
							Result = FALSE;
					}
				}
				else
				{
					if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
						SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);

					if (!DeleteFile(FileName))
						Result = FALSE;
				}
			}
		}
		while (FindNextFile(hFind, &FindData)!=0);

		FindClose(hFind);
	}

	// Remove directory
	if (Result)
		Result = RemoveDirectory(lpPath);

	return Result;
}
