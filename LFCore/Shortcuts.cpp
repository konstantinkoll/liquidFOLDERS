
#include "stdafx.h"
#include "LFCore.h"
#include "Mutex.h"
#include "Shortcuts.h"
#include "StoreCache.h"
#include "Stores.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern OSVERSIONINFO osInfo;


LFCORE_API BOOL LFAskCreateShortcut(HWND hwnd)
{
	if (osInfo.dwMajorVersion<6)
	{
		// Ask if link should be created on desktop
		WCHAR Caption[256];
		WCHAR Message[256];
		LoadString(LFCoreModuleHandle, IDS_SHORTCUTCAPTION, Caption, 256);
		LoadString(LFCoreModuleHandle, IDS_SHORTCUTMESSAGE, Message, 256);

		if (MessageBox(hwnd, Message, Caption, MB_YESNO | MB_ICONQUESTION)==IDNO)
			return FALSE;
	}

	return TRUE;
}

LFCORE_API void LFCreateDesktopShortcut(IShellLink* pShellLink, WCHAR* LinkFilename)
{
	// Get the fully qualified file name for the link file
	WCHAR PathDesktop[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, PathDesktop, CSIDL_DESKTOPDIRECTORY, FALSE))
	{
		WCHAR SanitizedLinkFilename[MAX_PATH];
		SanitizeFileName(SanitizedLinkFilename, MAX_PATH, LinkFilename);

		WCHAR PathLink[2*MAX_PATH];
		WCHAR NumberStr[16] = L"";
		UINT Number = 1;

		// Check if link file exists; otherwise append number
		do
		{
			wcscpy_s(PathLink, 2*MAX_PATH, PathDesktop);
			wcscat_s(PathLink, 2*MAX_PATH, L"\\");
			wcscat_s(PathLink, 2*MAX_PATH, SanitizedLinkFilename);
			wcscat_s(PathLink, 2*MAX_PATH, NumberStr);
			wcscat_s(PathLink, 2*MAX_PATH, L".lnk");

			swprintf(NumberStr, 16, L" (%u)", ++Number);
		}
		while (_waccess(PathLink, 0)==0);

		// Query IShellLink for the IPersistFile interface for saving the 
		// shortcut in persistent storage
		IPersistFile* pPersistFile = NULL;
		if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile)))
		{
			// Save the link by calling IPersistFile::Save
			pPersistFile->Save(PathLink, TRUE);
			pPersistFile->Release();
		}
	}
}

BOOL GetStoreManagerPath(WCHAR* Path, SIZE_T cCount)
{
	// Registry
	HKEY k;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\liquidFOLDERS\\", &k)==ERROR_SUCCESS)
	{
		DWORD sz = (DWORD)(cCount*sizeof(WCHAR));
		LSTATUS Result = RegQueryValueEx(k, L"StoreManager", 0, NULL, (BYTE*)Path, &sz);

		RegCloseKey(k);

		if (Result==ERROR_SUCCESS)
			return TRUE;
	}

	// Festen Pfad probieren
	if (!SHGetSpecialFolderPath(NULL, Path, CSIDL_PROGRAM_FILES, FALSE))
		return FALSE;

	wcscat_s(Path, cCount, L"\\liquidFOLDERS\\StoreManager.exe");
	return (_waccess(Path, 0)==0);
}

IShellLink* GetShortcutForStore(CHAR* StoreID, UINT IconID)
{
	WCHAR Path[2*MAX_PATH];
	if (GetStoreManagerPath(Path, 2*MAX_PATH))
	{
		// Get a pointer to the IShellLink interface
		IShellLink* pShellLink = NULL;
		if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
		{
			WCHAR ID[LFKeySize];
			MultiByteToWideChar(CP_ACP, 0, StoreID, -1, ID, LFKeySize);
	
			WCHAR IconLocation[2*MAX_PATH];
			GetModuleFileName(LFCoreModuleHandle, IconLocation, MAX_PATH);

			pShellLink->SetPath(Path);
			pShellLink->SetArguments(ID);
			pShellLink->SetIconLocation(IconLocation, IconID-1);
			pShellLink->SetShowCmd(SW_SHOWNORMAL);

			return pShellLink;
		}
	}

	return NULL;
}


LFCORE_API IShellLink* LFGetShortcutForStore(LFItemDescriptor* i)
{
	assert(i);

	return (i->Type & LFTypeStore) ? GetShortcutForStore(i->StoreID, i->IconID) : NULL;
}

LFCORE_API IShellLink* LFGetShortcutForStore(LFStoreDescriptor* s)
{
	assert(s);

	LFItemDescriptor* i = LFAllocItemDescriptor(s);
	IShellLink* pShellLink = LFGetShortcutForStore(i);
	LFFreeItemDescriptor(i);

	return pShellLink;
}

LFCORE_API IShellLink* LFGetShortcutForStore(CHAR* key)
{
	if (!key)
		return NULL;

	if (!GetMutex(Mutex_Stores))
		return NULL;

	LFStoreDescriptor* slot = FindStore(key[0]=='\0' ? DefaultStore : key);
	IShellLink* pShellLink = slot ? LFGetShortcutForStore(slot) : NULL;

	ReleaseMutex(Mutex_Stores);

	return pShellLink;
}


LFCORE_API void LFCreateDesktopShortcutForStore(LFItemDescriptor* i)
{
	IShellLink* pShellLink = LFGetShortcutForStore(i);
	if (pShellLink)
	{
		LFCreateDesktopShortcut(pShellLink, i->CoreAttributes.FileName);
		pShellLink->Release();
	}
}

LFCORE_API void LFCreateDesktopShortcutForStore(LFStoreDescriptor* s)
{
	IShellLink* pShellLink = LFGetShortcutForStore(s);
	if (pShellLink)
	{
		LFCreateDesktopShortcut(pShellLink, s->StoreName);
		pShellLink->Release();
	}
}

LFCORE_API void LFCreateDesktopShortcutForStore(CHAR* key)
{
	if (!key)
		return;

	if (!GetMutex(Mutex_Stores))
		return;

	LFStoreDescriptor* slot = FindStore(key[0]=='\0' ? DefaultStore : key);
	if (slot)
		LFCreateDesktopShortcutForStore(slot);

	ReleaseMutex(Mutex_Stores);
}

