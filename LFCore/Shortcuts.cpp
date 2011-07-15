
#include "stdafx.h"
#include "Shortcuts.h"
#include "LFCore.h"
#include "Mutex.h"
#include "StoreCache.h"
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
		wchar_t PathLink[2*MAX_PATH];
		wchar_t NumberStr[16] = L"";
		int Number = 1;

		// Check if link file exists; otherwise append number
		do
		{
			wcscpy_s(PathLink, 2*MAX_PATH, PathDesktop);
			wcscat_s(PathLink, 2*MAX_PATH, L"\\");
			wcscat_s(PathLink, 2*MAX_PATH, LinkFilename);
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
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\liquidFOLDERS\\", &k)==ERROR_SUCCESS)
	{
		DWORD sz = cCount*sizeof(wchar_t);
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

void CreateShortcutForStore(wchar_t* Name, char* StoreID, unsigned int IconID)
{
	wchar_t Path[2*MAX_PATH];
	if (!GetStoreManagerPath(Path, 2*MAX_PATH))
		return;

	// Get a pointer to the IShellLink interface
	IShellLink* pShellLink = NULL;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
	{
		wchar_t ID[LFKeySize];
		MultiByteToWideChar(CP_ACP, 0, StoreID, -1, ID, LFKeySize);

		pShellLink->SetPath(Path);
		pShellLink->SetArguments(ID);
		pShellLink->SetIconLocation(L"LFCORE.DLL", (IconID==IDI_STORE_Default ? IDI_STORE_Internal : IconID)-1);
		pShellLink->SetShowCmd(SW_SHOWNORMAL);

		LFCreateDesktopShortcut(pShellLink, Name);

		pShellLink->Release();
	}
}

LFCore_API void LFCreateShortcutForStore(LFItemDescriptor* i)
{
	assert(i);

	if (i->Type & LFTypeStore)
		CreateShortcutForStore(i->CoreAttributes.FileName, i->StoreID, i->IconID);
}

LFCore_API void LFCreateShortcutForStore(LFStoreDescriptor* s)
{
	assert(s);

	LFItemDescriptor* i = LFAllocItemDescriptor(s);
	LFCreateShortcutForStore(i);
	LFFreeItemDescriptor(i);
}

LFCore_API void LFCreateShortcutForStore(char* key)
{
	if (!key)
		return;

	if (!GetMutex(Mutex_Stores))
		return;

	LFStoreDescriptor* slot = FindStore(key[0]=='\0' ? DefaultStore : key);
	if (slot)
		LFCreateShortcutForStore(slot);

	ReleaseMutex(Mutex_Stores);
}
