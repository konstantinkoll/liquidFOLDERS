
#include "stdafx.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "Shortcuts.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;


LFCORE_API void LFCreateDesktopShortcut(IShellLink* pShellLink, WCHAR* pLinkFileName)
{
	// Get the fully qualified file name for the link file
	WCHAR PathDesktop[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, PathDesktop, CSIDL_DESKTOPDIRECTORY, FALSE))
	{
		WCHAR SanitizedLinkFileName[MAX_PATH];
		SanitizeFileName(SanitizedLinkFileName, MAX_PATH, pLinkFileName);

		WCHAR PathLink[2*MAX_PATH];
		WCHAR NumberStr[16] = L"";
		UINT Number = 1;

		// Check if link file exists; if yes append number
		do
		{
			wcscpy_s(PathLink, 2*MAX_PATH, PathDesktop);
			wcscat_s(PathLink, 2*MAX_PATH, L"\\");
			wcscat_s(PathLink, 2*MAX_PATH, SanitizedLinkFileName);
			wcscat_s(PathLink, 2*MAX_PATH, NumberStr);
			wcscat_s(PathLink, 2*MAX_PATH, L".lnk");

			swprintf_s(NumberStr, 16, L" (%u)", ++Number);
		}
		while (_waccess(PathLink, 0)==0);

		// Query IShellLink for the IPersistFile interface to save the shortcut in persistent storage
		IPersistFile* pPersistFile;
		if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile)))
		{
			// Save the link by calling IPersistFile::Save
			pPersistFile->Save(PathLink, TRUE);
			pPersistFile->Release();
		}
	}
}

IShellLink* GetShortcutForStore(CHAR* StoreID, WCHAR* Comments, UINT IconID)
{
	WCHAR Path[MAX_PATH];
	if (LFGetApplicationPath(Path, MAX_PATH))
	{
		// Get a pointer to the IShellLink interface
		IShellLink* pShellLink = NULL;
		if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
		{
			WCHAR Arguments[LFKeySize];
			MultiByteToWideChar(CP_ACP, 0, StoreID, -1, Arguments, LFKeySize);
	
			WCHAR IconLocation[MAX_PATH];
			GetModuleFileName(LFCoreModuleHandle, IconLocation, MAX_PATH);

			pShellLink->SetPath(Path);
			pShellLink->SetArguments(Arguments);
			pShellLink->SetDescription(Comments);
			pShellLink->SetIconLocation(IconLocation, IconID-1);
			pShellLink->SetShowCmd(SW_SHOWNORMAL);

			return pShellLink;
		}
	}

	return NULL;
}

LFCORE_API IShellLink* LFGetShortcutForStore(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	return (pItemDescriptor->Type & LFTypeMask)==LFTypeStore ? GetShortcutForStore(pItemDescriptor->StoreID, pItemDescriptor->CoreAttributes.Comments, pItemDescriptor->IconID) : NULL;
}

LFCORE_API void LFCreateDesktopShortcutForStore(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	IShellLink* pShellLink = LFGetShortcutForStore(pItemDescriptor);
	if (pShellLink)
	{
		LFCreateDesktopShortcut(pShellLink, pItemDescriptor->CoreAttributes.FileName);

		pShellLink->Release();
	}
}

LFCORE_API void LFCreateDesktopShortcutForStoreEx(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	IShellLink* pShellLink = GetShortcutForStore(pStoreDescriptor->StoreID, pStoreDescriptor->Comments, LFGetStoreIcon(pStoreDescriptor));
	if (pShellLink)
	{
		LFCreateDesktopShortcut(pShellLink, pStoreDescriptor->StoreName);

		pShellLink->Release();
	}
}
