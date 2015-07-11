
#include "stdafx.h"
#include "LF.h"
#include <assert.h>
#include <shlobj.h>


__forceinline LPITEMIDLIST AllocPIDL(SIZE_T cb)
{
	return (LPITEMIDLIST)CoTaskMemAlloc(cb);
}

__forceinline LPITEMIDLIST Next(LPITEMIDLIST pidl)
{
	return (LPITEMIDLIST)((BYTE*)pidl+pidl->mkid.cb);
}

SIZE_T GetSize(LPITEMIDLIST pidl)
{
	if (!pidl)
		return 0;

	SIZE_T Size = sizeof(pidl->mkid.cb);

	while (pidl->mkid.cb)
	{
		Size += pidl->mkid.cb;
		pidl = Next(pidl);
	}

	return Size;
}

LPITEMIDLIST ConcatenatePIDLs(LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
	SIZE_T cb1 = GetSize(pidl1)-sizeof(USHORT);
	SIZE_T cb2 = GetSize(pidl2);

	LPITEMIDLIST pidlNew = AllocPIDL(cb1+cb2);
	if (pidlNew)
	{
		memcpy(pidlNew, pidl1, cb1);
		memcpy((BYTE*)pidlNew+cb1, pidl2, cb2);
	}

	return pidlNew;
}

BOOL GetPIDLsForStore(CHAR* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate)
{
	assert(ppidl);
	assert(ppidlDelegate);

	*ppidl = *ppidlDelegate = NULL;

	// Path
	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, L"::{3F2D914F-FE57-414F-9F88-A377C7841DA4}");

	if (StoreID)
	{
		const SIZE_T cCount = 41;

		wcscat_s(Path, MAX_PATH, L"\\");
		MultiByteToWideChar(CP_ACP, 0, StoreID, -1, &Path[cCount], MAX_PATH-cCount);
	}

	// Main PIDL
	BOOL Result = SUCCEEDED(SHParseDisplayName(Path, NULL, ppidl, SFGAO_FOLDER, NULL));

	// Delegate PIDL
	LPITEMIDLIST pidlMyComputer;
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer)))
	{
		if (!StoreID)
		{
			*ppidlDelegate = pidlMyComputer;
			Result = TRUE;
		}
		else
		{
			IShellFolder* pDesktop;
			if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
			{
				IShellFolder* pParentFolder;
				if (SUCCEEDED(pDesktop->BindToObject(pidlMyComputer, NULL, IID_IShellFolder, (void**)&pParentFolder)))
				{
					IEnumIDList* pEnum;
					if (SUCCEEDED(pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
					{
						LPITEMIDLIST pidlTemp;
						while (pEnum->Next(1, &pidlTemp, NULL)==S_OK)
						{
							SHDESCRIPTIONID did;
							if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &did, sizeof(SHDESCRIPTIONID))))
							{
								static const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
								if (did.clsid==LFNE)
								{
									STRRET Name;
									if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pidlTemp, SHGDN_FORPARSING, &Name)))
										if (wcscmp(Name.pOleStr, Path)==0)
										{
											*ppidlDelegate = ConcatenatePIDLs(pidlMyComputer, pidlTemp);
											Result = TRUE;

											break;
										}
								}
							}
						}

						pEnum->Release();
					}

					pParentFolder->Release();
				}

				pDesktop->Release();
			}

			CoTaskMemFree(pidlMyComputer);
		}
	}

	return Result;
}
