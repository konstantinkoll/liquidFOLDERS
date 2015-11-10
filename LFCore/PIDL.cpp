
#include "stdafx.h"
#include "LF.h"
#include "ShellProperties.h"
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

BOOL GetPIDLsForStore(const CHAR* pStoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate)
{
	assert(ppidl);
	assert(ppidlDelegate);

	*ppidl = *ppidlDelegate = NULL;

	// Path
	WCHAR Path[MAX_PATH];
	wcscpy_s(Path, MAX_PATH, L"::{3F2D914F-FE57-414F-9F88-A377C7841DA4}");

	if (pStoreID)
	{
		const SIZE_T cCount = 41;

		wcscat_s(Path, MAX_PATH, L"\\");
		MultiByteToWideChar(CP_ACP, 0, pStoreID, -1, &Path[cCount], MAX_PATH-cCount);
	}

	// Main PIDL
	BOOL Result = SUCCEEDED(SHParseDisplayName(Path, NULL, ppidl, SFGAO_FOLDER, NULL));

	// Delegate PIDL
	LPITEMIDLIST pidlMyComputer;
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer)))
	{
		if (!pStoreID)
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
							SHDESCRIPTIONID SHDID;
							if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &SHDID, sizeof(SHDESCRIPTIONID))))
								if (SHDID.clsid==PropertyLiquidFolders)
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

void SendShellNotifyMessage(UINT Msg, const CHAR* pStoreID, LPITEMIDLIST pidlOld, LPITEMIDLIST pidlOldDelegate)
{
	LPITEMIDLIST pidl;
	LPITEMIDLIST pidlDelegate;
	if (GetPIDLsForStore(pStoreID, &pidl, &pidlDelegate))
	{
		SHChangeNotify(Msg, SHCNF_IDLIST | SHCNF_FLUSH, pidlOld ? pidlOld : pidl, (Msg==SHCNE_RENAMEFOLDER) ? pidl : NULL);
		CoTaskMemFree(pidl);

		if (pidlDelegate)
		{
			SHChangeNotify(Msg, SHCNF_IDLIST | SHCNF_FLUSH, pidlOldDelegate ? pidlOldDelegate : pidlDelegate, (Msg==SHCNE_RENAMEFOLDER) ? pidlDelegate : NULL);
			CoTaskMemFree(pidlDelegate);
		}
	}
}
