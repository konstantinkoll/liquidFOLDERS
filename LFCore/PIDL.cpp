#include "StdAfx.h"
#include "liquidFOLDERS.h"
#include <shellapi.h>
#include <shlobj.h>


LPITEMIDLIST AllocPIDL(unsigned int sz)
{
	LPMALLOC pMalloc;
	if (FAILED(SHGetMalloc(&pMalloc)))
		return NULL;

	LPITEMIDLIST pidl = (LPITEMIDLIST)pMalloc->Alloc(sz);
	if (pidl)
		ZeroMemory(pidl, sz);

	pMalloc->Release();
	return pidl;
}

void FreePIDL(LPITEMIDLIST pidl)
{
	if (pidl)
	{
		LPMALLOC pMalloc;
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
	}
}

LPITEMIDLIST Next(LPITEMIDLIST pidl)
{
	LPBYTE lpMem = (LPBYTE)pidl;
	lpMem += pidl->mkid.cb;

	return (LPITEMIDLIST)lpMem;
}

unsigned int GetSize(LPITEMIDLIST pidl)
{
	if (!pidl)
		return 0;

	unsigned int sz = sizeof(pidl->mkid.cb);

	while(pidl->mkid.cb)
	{
		sz += pidl->mkid.cb;
		pidl = Next(pidl);
	}

	return sz;
}

LPITEMIDLIST ConcatenatePIDLs(LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
	unsigned int cb1 = pidl1 ? GetSize(pidl1)-sizeof(pidl1->mkid.cb) : 0;
	unsigned int cb2 = GetSize(pidl2);

	LPITEMIDLIST pidlNew = AllocPIDL(cb1+cb2);
	if (pidlNew)
	{
		if (pidl1)
			memcpy(pidlNew, pidl1, cb1);
		memcpy(((LPSTR)pidlNew)+cb1, pidl2, cb2);
	}

	return pidlNew;
}

bool GetPIDLForStore(char* StoreID, LPITEMIDLIST* ppidl, LPITEMIDLIST* ppidlDelegate)
{
	*ppidl = *ppidlDelegate = NULL;

	IShellFolder* pDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return false;

	// Path
	wchar_t Key[LFKeySize+1];
	if (StoreID)
	{
		wcscpy_s(Key, LFKeySize+1, L"\\");
		MultiByteToWideChar(CP_ACP, 0, StoreID, -1, &Key[1], LFKeySize);
	}
	else
	{
		Key[0] = L'\0';
	}

	wchar_t Path[MAX_PATH];
	ULONG chEaten = 0;
	ULONG dwAttributes = SFGAO_FOLDER;

	wcscpy_s(Path, MAX_PATH, L"::{3F2D914F-FE57-414F-9F88-A377C7841DA4}");
	wcscat_s(Path, MAX_PATH, Key);

	// PIDL
	bool res = SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path, &chEaten, ppidl, &dwAttributes));

	// Delegate PIDL
	if (res)
	{
		LPITEMIDLIST pidlMyComputer;
		if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlMyComputer)))
		{
			if (!StoreID)
			{
				*ppidlDelegate = pidlMyComputer;

				pDesktop->Release();
				return true;
			}

			IShellFolder* pParentFolder = NULL;
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
							const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
							if (did.clsid==LFNE)
							{
								STRRET Name;
								if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pidlTemp, SHGDN_FORPARSING, &Name)))
									if (wcscmp(Name.pOleStr, Path)==0)
									{
										*ppidlDelegate = ConcatenatePIDLs(pidlMyComputer, pidlTemp);
										break;
									}
							}
						}
					}

					pEnum->Release();
				}
				pParentFolder->Release();
			}
		}
	}

	pDesktop->Release();
	return res;
}
