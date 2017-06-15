
#include "stdafx.h"
#include "Cloud.h"
#include "FileSystem.h"
#include "LFCore.h"
#include <assert.h>
#include <shlobj.h>


extern OSVERSIONINFO osInfo;


// {A52BBA46-E9E1-435F-B3D9-28DAA648C0F6}
static const GUID FOLDERID_OneDrive = { 0xA52BBA46, 0xE9E1, 0x435F, { 0xB3, 0xD9, 0x28, 0xDA, 0xA6, 0x48, 0xC0, 0xF6 } };

// {767E6811-49CB-4273-87C2-20F355E1085B}
static const GUID FOLDERID_OneDriveCameraRoll = { 0x767E6811, 0x49CB, 0x4273, { 0x87, 0xC2, 0x20, 0xF3, 0x55, 0xE1, 0x08, 0x5B } };

// {24D89E24-2F19-4534-9DDE-6A6671FBB8FE}
static const GUID FOLDERID_OneDriveDocuments = { 0x24D89E24, 0x2F19, 0x4534, { 0x9D, 0xDE, 0x6A, 0x66, 0x71, 0xFB, 0xB8, 0xFE } };

// {339719B5-8C47-4894-94C2-D8F77ADD44A6}
static const GUID FOLDERID_OneDrivePictures = { 0x339719B5, 0x8C47, 0x4894, { 0x94, 0xC2, 0xD8, 0xF7, 0x7A, 0xDD, 0x44, 0xA6 } };


BOOL GetProfilePath(LPWSTR pPath, LPCWSTR pFolder)
{
	assert(pPath);

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, pPath)))
	{
		wcscat_s(pPath, MAX_PATH, pFolder);

		if (DirectoryExists(pPath))
			return TRUE;
	}

	*pPath = L'\0';

	return FALSE;
}

LFCORE_API BOOL LFGetBoxPath(LPWSTR pPath)
{
	assert(pPath);

	return GetProfilePath(pPath, L"\\Box Sync");
}

LFCORE_API BOOL LFGetICloudPath(LPWSTR pPath)
{
	assert(pPath);

	return GetProfilePath(pPath, L"\\iCloudDrive");
}

HRESULT GetKnownFolderPath(REFKNOWNFOLDERID rfid, LPWSTR lpPath)
{
	if (!lpPath)
		return E_POINTER;

	*lpPath = L'\0';

	typedef HRESULT (__stdcall* PFNSHGETKNOWNFOLDERPATH)(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR* ppszPath);
	PFNSHGETKNOWNFOLDERPATH zSHGetKnownFolderPath = (PFNSHGETKNOWNFOLDERPATH)GetProcAddress(GetModuleHandle(L"SHELL32.DLL"), "SHGetKnownFolder");

	if (zSHGetKnownFolderPath)
	{
		PWSTR pComStr;
		HRESULT hResult = zSHGetKnownFolderPath(rfid, 0, NULL, &pComStr);
		if (SUCCEEDED(hResult))
		{
			wcscpy_s(lpPath, MAX_PATH, pComStr);
			CoTaskMemFree(pComStr);
		}

		return hResult;
	}

	return E_NOTIMPL;
}

void AddOneDrivePaths(LFOneDrivePaths& OneDrivePaths)
{
	assert(OneDrivePaths.OneDrive[0]!=L'\0');

#define TESTPATH(Dst, Suffix) \
	wcscpy_s(Dst, MAX_PATH, OneDrivePaths.OneDrive); \
	wcscat_s(Dst, MAX_PATH, Suffix); \
	if (!DirectoryExists(Dst)) \
		Dst[0] = L'\0';

	TESTPATH(OneDrivePaths.CameraRoll, L"\\Pictures\\Camera Roll");
	TESTPATH(OneDrivePaths.Documents, L"\\Documents");
	TESTPATH(OneDrivePaths.Pictures, L"\\Pictures");
}

LFCORE_API BOOL LFGetOneDrivePaths(LFOneDrivePaths& OneDrivePaths)
{
	// Windows 8.1 and newer
	if ((osInfo.dwMajorVersion>6) || ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion>=3)))
		if (SUCCEEDED(GetKnownFolderPath(FOLDERID_OneDrive, OneDrivePaths.OneDrive)))
		{
			GetKnownFolderPath(FOLDERID_OneDriveCameraRoll, OneDrivePaths.CameraRoll);
			GetKnownFolderPath(FOLDERID_OneDriveDocuments, OneDrivePaths.Documents);
			GetKnownFolderPath(FOLDERID_OneDrivePictures, OneDrivePaths.Pictures);

			return TRUE;
		}

	ZeroMemory(&OneDrivePaths, sizeof(OneDrivePaths));

	// Fallback: Windows Registry
	HKEY hKey;
	if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\OneDrive", &hKey)==ERROR_SUCCESS)
	{
		DWORD Size = sizeof(OneDrivePaths.OneDrive);
		BOOL Result = (RegQueryValueEx(hKey, L"UserFolder", 0, NULL, (BYTE*)OneDrivePaths.OneDrive, &Size)==ERROR_SUCCESS);

		RegCloseKey(hKey);

		if (Result)
			if (DirectoryExists(OneDrivePaths.OneDrive))
			{
				AddOneDrivePaths(OneDrivePaths);

				return TRUE;
			}
	}

	// Environment
	if (GetEnvironmentVariable(L"OneDrive", OneDrivePaths.OneDrive, MAX_PATH))
		if (DirectoryExists(OneDrivePaths.OneDrive))
		{
			AddOneDrivePaths(OneDrivePaths);

			return TRUE;
		}

	// Fail
	return FALSE;
}
