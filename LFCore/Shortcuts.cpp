
#include "stdafx.h"
#include "Shortcuts.h"
#include "LFCore.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;


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
