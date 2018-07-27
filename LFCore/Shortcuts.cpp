
#include "stdafx.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "Shortcuts.h"


extern HMODULE LFCoreModuleHandle;


UINT GetShortcutForStore(IShellLink*& pShellLink, const ABSOLUTESTOREID& StoreID, LPCWSTR Comments, UINT IconID)
{
	assert(IconID>0);

	WCHAR Path[MAX_PATH];
	if (LFGetApplicationPath(Path, MAX_PATH))
		if (LFCreateShellLink(pShellLink))
		{
			// Physical path to file
			pShellLink->SetPath(Path);

			// Arguments for app invoke
			WCHAR Arguments[LFKeySize];
			MultiByteToWideChar(CP_ACP, 0, StoreID, -1, Arguments, LFKeySize);

			pShellLink->SetArguments(Arguments);

			// Comments
			if (Comments)
				pShellLink->SetDescription(Comments);

			// Icon
			WCHAR IconLocation[MAX_PATH];
			GetModuleFileName(LFCoreModuleHandle, IconLocation, MAX_PATH);
			
			pShellLink->SetIconLocation(IconLocation, IconID-1);

			return LFOk;
		}

	return LFRegistryError;
}

__forceinline UINT GetShortcutForFile(IShellLink*& pShellLink, LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	UINT Result;
	WCHAR Path[MAX_PATH];
	if ((Result=LFGetFileLocation(pItemDescriptor, Path, MAX_PATH))!=LFOk)
		return Result;

	if (LFCreateShellLink(pShellLink))
	{
		pShellLink->SetPath(Path);

		return LFOk;
	}

	return LFRegistryError;
}

LFCORE_API UINT LFGetShortcutForItem(LFItemDescriptor* pItemDescriptor, IShellLink*& pShellLink)
{
	assert(pItemDescriptor);

	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeStore:
		return GetShortcutForStore(pShellLink, pItemDescriptor->StoreID, pItemDescriptor->CoreAttributes.Comments, pItemDescriptor->IconID);

	case LFTypeFile:
		return GetShortcutForFile(pShellLink, pItemDescriptor);
	}

	return LFIllegalItemType;
}

UINT CreateDesktopShortcut(IShellLink* pShellLink, LPCWSTR pLinkFileName)
{
	assert(pShellLink);
	assert(pLinkFileName);

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
		if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile)))
		{
			// Save the link by calling IPersistFile::Save
			pPersistFile->Save(PathLink, TRUE);
			pPersistFile->Release();

			return LFOk;
		}

		return LFRegistryError;
	}

	return LFIllegalPhysicalPath;
}


LFCORE_API UINT LFCreateDesktopShortcutForItem(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	IShellLink* pShellLink;
	UINT Result = LFGetShortcutForItem(pItemDescriptor, pShellLink);
	if (Result!=LFOk)
		return Result;

	// Save on desktop
	Result = CreateDesktopShortcut(pShellLink, pItemDescriptor->CoreAttributes.FileName);
	pShellLink->Release();

	return Result;
}

LFCORE_API UINT LFCreateDesktopShortcutForStore(const LFStoreDescriptor& StoreDescriptor)
{
	IShellLink* pShellLink;
	UINT Result = GetShortcutForStore(pShellLink, StoreDescriptor.StoreID, StoreDescriptor.Comments, LFGetStoreIcon(&StoreDescriptor));
	if (Result!=LFOk)
		return Result;

	// Save on desktop
	Result = CreateDesktopShortcut(pShellLink, StoreDescriptor.StoreName);
	pShellLink->Release();

	return Result;
}
