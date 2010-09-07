
// CPIDLDropdownSelector.cpp: Implementierung der Klasse CPIDLDropdownSelector
//

#include "stdafx.h"
#include "CPIDLSelector.h"
#include "Migrate.h"
#include "resource.h"


// CPIDLDropdownWindow
//

CPIDLDropdownWindow::CPIDLDropdownWindow()
	: CDropdownWindow()
{
}

BOOL CPIDLDropdownWindow::AddPIDL(LPITEMIDLIST pidl, UINT Category, BOOL FreeOnFail)
{
	if (!pidl)
		return FALSE;
	SHFILEINFO sfi;
	if (FAILED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX)))
	{
		if (FreeOnFail)
			theApp.GetShellManager()->FreeItem(pidl);
		return FALSE;
	}
	if (!sfi.dwAttributes)
	{
		if (FreeOnFail)
			theApp.GetShellManager()->FreeItem(pidl);
		return FALSE;
	}

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID;
	lvi.iItem = m_wndList.GetItemCount();
	lvi.pszText = sfi.szDisplayName;
	lvi.iImage = sfi.iIcon;
	lvi.iGroupId = Category;
	m_wndList.SetItemData(m_wndList.InsertItem(&lvi), (DWORD_PTR)pidl);

	return TRUE;
}

void CPIDLDropdownWindow::AddKnownFolder(REFKNOWNFOLDERID rfid, UINT Category)
{
	// Call only on Windows 7 !
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

		Desktop->Release();
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
	CShellManager* sm = theApp.GetShellManager();

	IShellFolder* Desktop;
	if (SUCCEEDED(SHGetDesktopFolder(&Desktop)))
	{
		ULONG chEaten;
		ULONG dwAttributes;
		LPITEMIDLIST pidl = NULL;
		Desktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidl, &dwAttributes);
		BOOL ParentAdded = AddPIDL(pidl, Category, FALSE);

		IShellFolder* Libraries;
		if (SUCCEEDED(Desktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&Libraries)))
		{
			IEnumIDList* pEnum;
			Libraries->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum);
			{
				LPITEMIDLIST lib;
				while (pEnum->Next(1, &lib, NULL)==S_OK)
				{
					AddPIDL(sm->ConcatenateItem(pidl, lib), Category);
					sm->FreeItem(lib);
				}
				pEnum->Release();
			}

			Libraries->Release();
		}

		Desktop->Release();
		if (!ParentAdded)
			sm->FreeItem(pidl);
	}
}

void CPIDLDropdownWindow::PopulateList()
{
	m_wndList.DeleteAllItems();

	// Special folders and libraries
	AddCSIDL(CSIDL_DESKTOP, 0);											// Desktop
	AddCSIDL(CSIDL_MYDOCUMENTS, 0);										// My documents
	AddCSIDL(CSIDL_MYMUSIC, 0);											// My music
	AddCSIDL(CSIDL_MYPICTURES, 0);										// My pictures
	AddCSIDL(CSIDL_MYVIDEO, 0);											// My videos

	AddCSIDL(CSIDL_COMMON_DOCUMENTS, 2);							// Common documents
	AddCSIDL(CSIDL_COMMON_MUSIC, 2);								// Common music
	AddCSIDL(CSIDL_COMMON_PICTURES, 2);								// Common pictures
	AddCSIDL(CSIDL_COMMON_VIDEO, 2);								// Common videos

	switch (((LFApplication*)AfxGetApp())->OSVersion)
	{
	case OS_XP:
	case OS_Vista:
		break;
	default:
		AddKnownFolder(FOLDERID_Contacts, 0);							// Contacts
		AddKnownFolder(FOLDERID_Downloads, 0);							// Downloads
		AddChildren(_T("::{031E4825-7B94-4DC3-B131-E946B44C8DD5}"), 1);	// Libraries
		break;
	}

	AddCSIDL(CSIDL_FAVORITES, 0);										// Favorites
	AddCSIDL(CSIDL_FONTS, 2);											// Fonts

	// Drives
	UINT Drives = LFGetLogicalDrives(LFGLD_External);

	for (wchar_t cDrive=L'A'; cDrive<=L'Z'; cDrive++, Drives>>=1)
	{
		if (!(Drives & 1))
			continue;

		wchar_t Drive[4] = L" :\\";
		Drive[0] = cDrive;
		AddPath(Drive, 3);
	}
}


BEGIN_MESSAGE_MAP(CPIDLDropdownWindow, CDropdownWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, 1, OnItemChanged)
	ON_BN_CLICKED(IDOK, OnChooseFolder)
END_MESSAGE_MAP()

int CPIDLDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDropdownWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_FOLDERCATEGORY1+a));
		AddCategory(a, tmpStr);
	}

	m_wndList.SetImageList(&theApp.m_SystemImageListLarge, LVSIL_NORMAL);
	m_wndList.EnableGroupView(TRUE);

	IMAGEINFO ii;
	theApp.m_SystemImageListLarge.GetImageInfo(0, &ii);
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
	for (int a=0; a<m_wndList.GetItemCount(); a++)
	{
		LPITEMIDLIST pidl = (LPITEMIDLIST)m_wndList.GetItemData(a);
		if (pidl)
			theApp.GetShellManager()->FreeItem(pidl);
	}

	CDropdownWindow::OnDestroy();
}

void CPIDLDropdownWindow::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		LPITEMIDLIST pidl = (LPITEMIDLIST)m_wndList.GetItemData(pNMListView->iItem);
		GetOwner()->SendMessage(WM_SETITEM, NULL, (LPARAM)pidl);
	}
}

void CPIDLDropdownWindow::OnChooseFolder()
{
	GetOwner()->PostMessage(WM_CLOSEDROPDOWN);
	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_SELECTROOT);
}


// CDropdownSelector
//

CPIDLSelector::CPIDLSelector()
	: CDropdownSelector()
{
	pidl = NULL;
}

CPIDLSelector::~CPIDLSelector()
{
	if (pidl)
		theApp.GetShellManager()->FreeItem(pidl);
}

void CPIDLSelector::CreateDropdownWindow()
{
	p_DropWindow = new CPIDLDropdownWindow();
	p_DropWindow->Create(this, IDD_CHOOSEFOLDER);
}

void CPIDLSelector::SetEmpty(BOOL Repaint)
{
	if (pidl)
	{
		theApp.GetShellManager()->FreeItem(pidl);
		pidl = NULL;
	}

	CDropdownSelector::SetEmpty(Repaint);
}

void CPIDLSelector::SetItem(LPITEMIDLIST _pidl, BOOL Repaint)
{
	if (pidl)
		theApp.GetShellManager()->FreeItem(pidl);

	pidl = theApp.GetShellManager()->CopyItem(_pidl);
	if (pidl)
	{
		SHFILEINFO sfi;
		if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON)))
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_FOLDER_CAPTION));
			CDropdownSelector::SetItem(tmpStr, sfi.hIcon, sfi.szDisplayName, Repaint);
		}
		else
		{
			SetEmpty();
		}
	}
	else
	{
		SetEmpty();
	}
}

void CPIDLSelector::GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint)
{
	TooltipDataFromPIDL(pidl, &theApp.m_SystemImageListLarge, hIcon, size, caption, hint);
}


BEGIN_MESSAGE_MAP(CPIDLSelector, CDropdownSelector)
	ON_MESSAGE(WM_SETITEM, OnSetItem)
END_MESSAGE_MAP()

LRESULT CPIDLSelector::OnSetItem(WPARAM /*wParam*/, LPARAM lParam)
{
	SetItem((LPITEMIDLIST)lParam);
	return NULL;
}
