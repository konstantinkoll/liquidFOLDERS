
#include "stdafx.h"
#include "LFCore.h"
#include "Mutex.h"
#include "Shortcuts.h"
#include "StoreCache.h"
#include "Stores.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;


LFCore_API bool LFAskCreateShortcut(HWND hwnd)
{
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	if (osInfo.dwMajorVersion<6)
	{
		// Ask if link should be created on desktop
		wchar_t caption[256];
		wchar_t message[256];
		LoadString(LFCoreModuleHandle, IDS_ShortcutCaption, caption, 256);
		LoadString(LFCoreModuleHandle, IDS_ShortcutMessage, message, 256);

		if (MessageBox(hwnd, message, caption, MB_YESNO | MB_ICONQUESTION)==IDNO)
			return FALSE;
	}

	return true;
}

LFCore_API void LFCreateDesktopShortcut(IShellLink* pShellLink, wchar_t* LinkFilename)
{
	// Get the fully qualified file name for the link file
	wchar_t PathDesktop[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, PathDesktop, CSIDL_DESKTOPDIRECTORY, FALSE))
	{
		wchar_t SanitizedLinkFilename[MAX_PATH];
		SanitizeFileName(SanitizedLinkFilename, MAX_PATH, LinkFilename);

		wchar_t PathLink[2*MAX_PATH];
		wchar_t NumberStr[16] = L"";
		int Number = 1;

		// Check if link file exists; otherwise append number
		do
		{
			wcscpy_s(PathLink, 2*MAX_PATH, PathDesktop);
			wcscat_s(PathLink, 2*MAX_PATH, L"\\");
			wcscat_s(PathLink, 2*MAX_PATH, SanitizedLinkFilename);
			wcscat_s(PathLink, 2*MAX_PATH, NumberStr);
			wcscat_s(PathLink, 2*MAX_PATH, L".lnk");

			swprintf(NumberStr, 16, L" (%d)", ++Number);
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

bool GetStoreManagerPath(wchar_t* Path, size_t cCount)
{
	// Registry
	HKEY k;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\liquidFOLDERS\\", &k)==ERROR_SUCCESS)
	{
		DWORD sz = (DWORD)(cCount*sizeof(wchar_t));
		LSTATUS res = RegQueryValueEx(k, L"StoreManager", 0, NULL, (BYTE*)Path, &sz);

		RegCloseKey(k);

		if (res==ERROR_SUCCESS)
			return true;
	}

	// Festen Pfad probieren
	if (!SHGetSpecialFolderPath(NULL, Path, CSIDL_PROGRAM_FILES, FALSE))
		return false;

	wcscat_s(Path, cCount, L"\\liquidFOLDERS\\StoreManager.exe");
	return (_waccess(Path, 0)==0);
}

IShellLink* GetShortcutForStore(char* StoreID, unsigned int IconID)
{
	wchar_t Path[2*MAX_PATH];
	if (GetStoreManagerPath(Path, 2*MAX_PATH))
	{
		// Get a pointer to the IShellLink interface
		IShellLink* pShellLink = NULL;
		if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
		{
			wchar_t ID[LFKeySize];
			MultiByteToWideChar(CP_ACP, 0, StoreID, -1, ID, LFKeySize);
	
			wchar_t IconLocation[2*MAX_PATH];
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


LFCore_API IShellLink* LFGetShortcutForStore(LFItemDescriptor* i)
{
	assert(i);

	return (i->Type & LFTypeStore) ? GetShortcutForStore(i->StoreID, i->IconID) : NULL;
}

LFCore_API IShellLink* LFGetShortcutForStore(LFStoreDescriptor* s)
{
	assert(s);

	LFItemDescriptor* i = LFAllocItemDescriptor(s);
	IShellLink* pShellLink = LFGetShortcutForStore(i);
	LFFreeItemDescriptor(i);

	return pShellLink;
}

LFCore_API IShellLink* LFGetShortcutForStore(char* key)
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


LFCore_API void LFCreateDesktopShortcutForStore(LFItemDescriptor* i)
{
	IShellLink* pShellLink = LFGetShortcutForStore(i);
	if (pShellLink)
	{
		LFCreateDesktopShortcut(pShellLink, i->CoreAttributes.FileName);
		pShellLink->Release();
	}
}

LFCore_API void LFCreateDesktopShortcutForStore(LFStoreDescriptor* s)
{
	IShellLink* pShellLink = LFGetShortcutForStore(s);
	if (pShellLink)
	{
		LFCreateDesktopShortcut(pShellLink, s->StoreName);
		pShellLink->Release();
	}
}

LFCore_API void LFCreateDesktopShortcutForStore(char* key)
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

