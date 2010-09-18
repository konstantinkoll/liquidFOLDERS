
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
			if (SUCCEEDED(Libraries->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
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

	m_wndList.SetImageList(&theApp.m_SystemImageListLarge, LVSIL_NORMAL);

	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_FOLDERCATEGORY1+a));
		m_wndList.AddCategory(a, tmpStr);
	}
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
		GetOwner()->SendMessage(WM_SETITEM, NULL, (LPARAM)m_wndList.GetItemData(pNMListView->iItem));
}

void CPIDLDropdownWindow::OnChooseFolder()
{
	GetOwner()->PostMessage(WM_CLOSEDROPDOWN);
	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_SELECTROOT);
}


// CPIDLSelector
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

void CPIDLSelector::SetItem(LPITEMIDLIST _pidl, BOOL Repaint, UINT NotifyCode)
{
	if (pidl)
		theApp.GetShellManager()->FreeItem(pidl);

	pidl = theApp.GetShellManager()->CopyItem(_pidl);
	if (pidl)
	{
		SHFILEINFO sfi;
		if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_OPENICON)))
		{
			CDropdownSelector::SetItem(sfi.hIcon, sfi.szDisplayName, Repaint, NotifyCode);
		}
		else
		{
			SetEmpty(Repaint);
		}
	}
	else
	{
		SetEmpty(Repaint);
	}
}

void CPIDLSelector::SetItem(IShellFolder* pDesktop, wchar_t* Path, BOOL Repaint, UINT NotifyCode)
{
	ULONG chEaten;
	ULONG dwAttributes;
	LPITEMIDLIST pidlFQ = NULL;
	if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidlFQ, &dwAttributes)))
	{
		SetItem(pidlFQ, Repaint, NotifyCode);
		p_App->GetShellManager()->FreeItem(pidlFQ);
	}
	else
	{
		SetEmpty(Repaint);
	}
}

void CPIDLSelector::GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint)
{
	TooltipDataFromPIDL(pidl, &theApp.m_SystemImageListLarge, hIcon, size, caption, hint);
}


BEGIN_MESSAGE_MAP(CPIDLSelector, CDropdownSelector)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SETITEM, OnSetItem)
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_MESSAGE_MAP()

int CPIDLSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDropdownSelector::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Benachrichtigung, wenn sich Items �ndern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
		SHCNE_DRIVEREMOVED | SHCNE_MEDIAREMOVED | SHCNE_RMDIR | SHCNE_RENAMEFOLDER | SHCNE_UPDATEITEM | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);

	return 0;
}

void CPIDLSelector::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	CDropdownSelector::OnDestroy();
}

LRESULT CPIDLSelector::OnSetItem(WPARAM /*wParam*/, LPARAM lParam)
{
	SetItem((LPITEMIDLIST)lParam);
	return NULL;
}

LRESULT CPIDLSelector::OnShellChange(WPARAM wParam, LPARAM lParam)
{
	if (m_IsEmpty)
		return NULL;

	wchar_t CurrentPath[MAX_PATH] = L"";
	if (!SHGetPathFromIDList(pidl, CurrentPath))
		return NULL;

	LPITEMIDLIST* pidls = (LPITEMIDLIST*)wParam;

	wchar_t Path1[MAX_PATH] = L"";
	wchar_t Path2[MAX_PATH] = L"";
	wchar_t Parent1[MAX_PATH] = L"";
	wchar_t Parent2[MAX_PATH] = L"";

	IShellFolder* pDesktop = NULL;
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
	{
		SHGetPathFromIDList(pidls[0], Path1);

		wcscpy_s(Parent1, MAX_PATH, Path1);
		wchar_t* last = wcsrchr(Parent1, L'\\');
		if (last>&Parent1[2])
			*last = '\0';

		if (pidls[1])
		{
			SHGetPathFromIDList(pidls[1], Path2);

			wcscpy_s(Parent2, MAX_PATH, Path2);
			last = wcsrchr(Parent2, L'\\');
			if (last>&Parent2[2])
				*last = '\0';
		}
	}

	switch (lParam)
	{
	case SHCNE_DRIVEREMOVED:
	case SHCNE_MEDIAREMOVED:
	case SHCNE_RMDIR:
		if (wcscmp(CurrentPath, Path1)==0)
			SetEmpty();
		break;
	case SHCNE_RENAMEFOLDER:
		if ((Path1[0]!='\0') && (Path2[0]!='\0') && (wcscmp(Path1, CurrentPath)==0))
			SetItem(pDesktop, Path2, TRUE, wcscmp(Path1, Path2) ? NM_SELCHANGED : NM_SELUPDATE);
		break;
	case SHCNE_UPDATEITEM:
		wcscpy_s(Path2, MAX_PATH, Parent1);
		wcscat_s(Path2, MAX_PATH, L"\\desktop.ini");
		if ((wcscmp(Path1, Path2)==0) && (wcscmp(Path2, CurrentPath)!=0))
			SetItem(pDesktop, Parent1, TRUE, NM_SELUPDATE);
		break;
	}

	pDesktop->Release();

	return NULL;
}
