
// CPIDLDropdownSelector.cpp: Implementierung der Klasse CPIDLDropdownSelector
//

#include "stdafx.h"
#include "CPIDLSelector.h"
#include "Migrate.h"
#include "resource.h"


LPITEMIDLIST GetNextItem(LPITEMIDLIST pidl)
{
	if (!pidl)
		return NULL;

	return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl)+pidl->mkid.cb);
}

UINT GetByteSize(LPITEMIDLIST pidl)
{
	if (!pidl)
		return 0;

	UINT Size = 0;
	while (pidl->mkid.cb)
	{
		Size += pidl->mkid.cb;
		pidl = GetNextItem(pidl);
	}

	return Size+sizeof(ITEMIDLIST);
}


// CPIDLDropdownWindow
//

CPIDLDropdownWindow::CPIDLDropdownWindow()
	: CDropdownWindow()
{
}

void CPIDLDropdownWindow::AddPIDL(LPITEMIDLIST pidl, UINT Category)
{
	if (!pidl)
		return;
	SHFILEINFO sfi;
	if (FAILED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX)))
		return;
	if (!sfi.dwAttributes)
		return;

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID;
	lvi.iItem = m_wndList.GetItemCount();
	lvi.pszText = sfi.szDisplayName;
	lvi.iImage = sfi.iIcon;
	lvi.iGroupId = Category;
	m_wndList.InsertItem(&lvi);
}

void CPIDLDropdownWindow::AddKnownFolder(REFKNOWNFOLDERID rfid, UINT Category)
{
	if (((LFApplication*)AfxGetApp())->OSVersion<OS_Seven)
		return;

	LPITEMIDLIST pidl;
	if (SUCCEEDED(SHGetKnownFolderIDList(rfid, 0, NULL, &pidl)))
		AddPIDL(pidl, Category);
}

void CPIDLDropdownWindow::AddPath(wchar_t* Path, UINT Category)
{
	IShellFolder* Desktop;
	if (SUCCEEDED(SHGetDesktopFolder(&Desktop)))
	{
		ULONG chEaten;
		ULONG dwAttributes;
		LPITEMIDLIST pidl = NULL;
		Desktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidl, &dwAttributes);
		AddPIDL(pidl, Category);
	}
}

void CPIDLDropdownWindow::AddCSIDL(int CSIDL, UINT Category)
{
	wchar_t Path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL, NULL, NULL, Path)))
		AddPath(Path, Category);
}

void CPIDLDropdownWindow::AddChildren(wchar_t* Path, UINT Category)
{
	LPMALLOC pMalloc;
	if (FAILED(SHGetMalloc(&pMalloc)))
		return;

	IShellFolder* Desktop;
	if (SUCCEEDED(SHGetDesktopFolder(&Desktop)))
	{
		ULONG chEaten;
		ULONG dwAttributes;
		LPITEMIDLIST pidl = NULL;
		Desktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidl, &dwAttributes);
		AddPIDL(pidl, Category);

		IShellFolder* Libraries;
		if (SUCCEEDED(Desktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&Libraries)))
		{
			IEnumIDList* e;
			Libraries->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &e);
			{
				LPITEMIDLIST librel;
				LPITEMIDLIST libabs;
				while (e->Next(1, &librel, NULL)==S_OK)
				{
					UINT cb1 = GetByteSize(pidl)-sizeof(ITEMIDLIST);
					UINT cb2 = GetByteSize(librel);

					libabs = (LPITEMIDLIST)pMalloc->Alloc(cb1+cb2);
					if (libabs)
					{
						ZeroMemory(libabs, cb1+cb2);
						CopyMemory(libabs, pidl, cb1);
						CopyMemory(((LPBYTE)libabs)+cb1, librel, cb2);
						AddPIDL(libabs, Category);
					}
				}
				e->Release();
			}
		}
	}

}

void CPIDLDropdownWindow::PopulateList()
{
	m_wndList.DeleteAllItems();
	BOOL IsSeven = ((LFApplication*)AfxGetApp())->OSVersion>=OS_Seven;

	// Special folders and libraries
	AddCSIDL(CSIDL_DESKTOP, 0);											// Desktop
	AddCSIDL(CSIDL_MYDOCUMENTS, 0);										// My documents
	AddCSIDL(CSIDL_MYMUSIC, 0);											// My music
	AddCSIDL(CSIDL_MYPICTURES, 0);										// My pictures
	AddCSIDL(CSIDL_MYVIDEO, 0);											// My videos
	AddKnownFolder(FOLDERID_Contacts, 0);								// Contacts
	AddKnownFolder(FOLDERID_Downloads, 0);								// Downloads
	if (IsSeven)
		AddChildren(_T("::{031E4825-7B94-4dc3-B131-E946B44C8DD5}"), 1);	// Libraries

	// Drives
	UINT Drives = LFGetLogicalDrives(LFGLD_External);

	for (wchar_t cDrive=L'A'; cDrive<=L'Z'; cDrive++, Drives>>=1)
	{
		if (!(Drives & 1))
			continue;

		wchar_t Drive[4] = L" :\\";
		Drive[0] = cDrive;
		AddPath(Drive, 2);
	}
}


BEGIN_MESSAGE_MAP(CPIDLDropdownWindow, CDropdownWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnChooseFolder)
END_MESSAGE_MAP()

int CPIDLDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDropdownWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	for (UINT a=0; a<3; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_FOLDERCATEGORY1+a));
		AddCategory(a, tmpStr);
	}

	SHFILEINFO shfi;
	il.Attach((HIMAGELIST)SHGetFileInfo(_T(""), NULL, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_LARGEICON | SHGFI_ICON));
	m_wndList.SetImageList(&il, LVSIL_NORMAL);
	m_wndList.EnableGroupView(TRUE);

	IMAGEINFO ii;
	il.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);
	m_wndList.SetIconSpacing(CXDropdownListIconSpacing, ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy"), 2).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	PopulateList();
	return 0;
}

void CPIDLDropdownWindow::OnDestroy()
{
	CDropdownWindow::OnDestroy();
	il.Detach();
}

void CPIDLDropdownWindow::OnChooseFolder()
{
	ShowWindow(SW_HIDE);

	GetOwner()->MessageBox(_T("Test"));

	GetOwner()->PostMessage(WM_CLOSEDROPDOWN);
}


// CDropdownSelector
//

CPIDLSelector::CPIDLSelector()
	: CDropdownSelector()
{
}

void CPIDLSelector::CreateDropdownWindow()
{
	p_DropWindow = new CPIDLDropdownWindow();
	p_DropWindow->Create(this, IDD_CHOOSEFOLDER);
}
