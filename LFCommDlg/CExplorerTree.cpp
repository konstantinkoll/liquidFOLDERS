
// CExplorerTree.cpp: Implementierung der Klasse CExplorerTree
//

#include "stdafx.h"
#include "LFCommDlg.h"


int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	LPAFX_SHELLITEMINFO pItem1 = (LPAFX_SHELLITEMINFO)lParam1;
	LPAFX_SHELLITEMINFO pItem2 = (LPAFX_SHELLITEMINFO)lParam2;

	HRESULT hr = pItem1->pParentFolder->CompareIDs(0, pItem1->pidlRel, pItem2->pidlRel);
	if (FAILED(hr))
		return 0;

	return (short)SCODE_CODE(GetScode(hr));
}


// CExplorerTree
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CExplorerTree::CExplorerTree()
	: CTreeCtrl()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = LFCommDlgDLL.hModule;

	if (!(::GetClassInfo(hInst, L"CExplorerTree", &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = NULL;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = L"CExplorerTree";

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_App = (LFApplication*)AfxGetApp();
	m_pContextMenu2 = NULL;
	m_Hover = FALSE;
	m_HoverItem = NULL;
	m_ExplorerStyle = FALSE;
	m_ulSHChangeNotifyRegister = NULL;
}

void CExplorerTree::PreSubclassWindow()
{
	m_TooltipCtrl.Create(this);

	if (p_App->OSVersion==OS_XP)
		ModifyStyle(0, TVS_HASLINES);

	SetImageList(&p_App->m_SystemImageListSmall, 0);

	// Benachrichtigung, wenn sich Items ändern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED |
			SHCNE_MKDIR | SHCNE_RMDIR | SHCNE_RENAMEFOLDER | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);
}

LPITEMIDLIST CExplorerTree::GetSelectedPIDL()
{
	HTREEITEM hItem = GetSelectedItem();
	if (!hItem)
		return NULL;

	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hItem;
	if (!GetItem(&tvItem))
		return NULL;

	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)tvItem.lParam;
	return pItem->pidlFQ;
}

BOOL CExplorerTree::GetSelectedPathA(LPSTR Path)
{
	LPITEMIDLIST pidl = GetSelectedPIDL();
	return pidl ? SHGetPathFromIDListA(pidl, Path) : FALSE;
}

BOOL CExplorerTree::GetSelectedPathW(LPWSTR Path)
{
	LPITEMIDLIST pidl = GetSelectedPIDL();
	return pidl ? SHGetPathFromIDListW(pidl, Path) : FALSE;
}

LRESULT CExplorerTree::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITMENUPOPUP:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (m_pContextMenu2)
		{
			m_pContextMenu2->HandleMenuMsg(message, wParam, lParam);
			return 0;
		}
	}

	return CTreeCtrl::WindowProc(message, wParam, lParam);
}

BOOL CExplorerTree::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEWHEEL:
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CTreeCtrl::PreTranslateMessage(pMsg);
}

CString CExplorerTree::OnGetItemText(LPAFX_SHELLITEMINFO pItem)
{
	ASSERT(pItem);

	SHFILEINFO sfi;
	if (SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
		return sfi.szDisplayName;

	return _T("?");
}

int CExplorerTree::OnGetItemIcon(LPAFX_SHELLITEMINFO pItem, BOOL bSelected)
{
	ASSERT(pItem);

	SHFILEINFO sfi;
	if (SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | (bSelected ? SHGFI_OPENICON : SHGFI_LINKOVERLAY)))
		return sfi.iIcon;

	return -1;
}

HTREEITEM CExplorerTree::InsertItem(IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, HTREEITEM hParent, BOOL children, LPITEMIDLIST pidlFQ)
{
	if (!pidlRel)
		return NULL;
	if (!pidlFQ)
		pidlFQ = p_App->GetShellManager()->CopyItem(pidlRel);

	if (pParentFolder)
		pParentFolder->AddRef();

	TV_ITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
	pItem->pidlRel = pidlRel;
	pItem->pidlFQ = pidlFQ;
	pItem->pParentFolder = pParentFolder;
	tvItem.lParam = (LPARAM)pItem;

	CString strItem = OnGetItemText(pItem);
	tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
	tvItem.iImage = OnGetItemIcon(pItem, FALSE);
	tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);
	tvItem.cChildren = children;

	TV_INSERTSTRUCT tvInsert;
	tvInsert.item = tvItem;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.hParent = hParent;

	return CTreeCtrl::InsertItem(&tvInsert);
}

HTREEITEM CExplorerTree::InsertItem(wchar_t* Path)
{
	IShellFolder* pDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return NULL;

	ULONG chEaten;
	ULONG dwAttributes;
	LPITEMIDLIST pidlFQ = NULL;
	pDesktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidlFQ, &dwAttributes);
	pDesktop->Release();

	IShellFolder* pParentFolder = NULL;
	LPCITEMIDLIST pidlRel = NULL;
	if (FAILED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
		return NULL;

	HTREEITEM hItem = InsertItem(pParentFolder, p_App->GetShellManager()->CopyItem(pidlRel), TVI_ROOT, TRUE, pidlFQ);

	if (pParentFolder)
		pParentFolder->Release();

	return hItem;
}

void CExplorerTree::PopulateTree()
{
	DeleteAllItems();

	LPITEMIDLIST pidl;
	HTREEITEM hItem = NULL;
	if (m_RootPath.IsEmpty())
	{
		if (FAILED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl)))
			return;

		hItem = InsertItem(NULL, pidl);
	}
	else
		if ((m_RootPath==CETR_InternalDrives) || (m_RootPath==CETR_ExternalDrives))
		{
			DWORD DrivesOnSystem = LFGetLogicalDrives(m_RootPath==CETR_InternalDrives ? LFGLD_Internal | LFGLD_Network : LFGLD_External);
			wchar_t szDriveRoot[] = L" :\\";
			BOOL First = TRUE;

			for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
			{
				if (!(DrivesOnSystem & 1))
					continue;

				szDriveRoot[0] = cDrive;

				SHFILEINFO sfi;
				if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES))
					if (sfi.dwAttributes)
					{
						HTREEITEM hItem = InsertItem(szDriveRoot);
						if (First)
						{
							Select(hItem, TVGN_CARET);
							First = FALSE;
						}
					}
			}
		}
		else
		{
			hItem = InsertItem(m_RootPath.GetBuffer());
			Select(hItem, TVGN_CARET);
		}

	if ((hItem) && (!(GetStyle() & TVS_LINESATROOT)))
		Expand(hItem, TVE_EXPAND);
}

HTREEITEM CExplorerTree::FindItem(IShellFolder* pDesktop, LPITEMIDLIST pidl)
{
	ASSERT(pDesktop);

	HTREEITEM hItem = GetRootItem();

	while (hItem)
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvItem.hItem = hItem;

		if (GetItem(&tvItem))
		{
			LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)tvItem.lParam;

			// Equal
			if (pDesktop->CompareIDs(NULL, pItem->pidlFQ, pidl)==0)
				return hItem;
		}

		hItem = GetNextSiblingItem(hItem);
	}

	return NULL;
}

void CExplorerTree::SetRootPath(CString RootPath)
{
	if (RootPath!=m_RootPath)
	{
		m_RootPath = RootPath;
		PopulateTree();
	}
}

void CExplorerTree::SetOnlyFilesystem(BOOL OnlyFilesystem)
{
	m_OnlyFilesystem = OnlyFilesystem;
}

BOOL CExplorerTree::GetChildItems(HTREEITEM hParentItem)
{
	CWaitCursor wait;

	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hParentItem;
	if (!GetItem(&tvItem))
		return FALSE;

	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)tvItem.lParam;
	IShellFolder* pParentFolder = NULL;

	HRESULT hr;
	if (!pItem->pParentFolder)
	{
		hr = SHGetDesktopFolder(&pParentFolder);
	}
	else
	{
		hr = pItem->pParentFolder->BindToObject(pItem->pidlRel, NULL, IID_IShellFolder, (void**)&pParentFolder);
	}

	if (FAILED(hr))
		return FALSE;

	SetRedraw(FALSE);

	EnumObjects(hParentItem, pParentFolder, pItem->pidlFQ);
	pParentFolder->Release();

	TV_SORTCB tvSort;
	tvSort.hParent = hParentItem;
	tvSort.lpfnCompare = CompareProc;
	tvSort.lParam = 0;
	SortChildrenCB(&tvSort);

	SetRedraw(TRUE);
	RedrawWindow();

	return TRUE;
}

void CExplorerTree::EnumObjects(HTREEITEM hParentItem, IShellFolder* pParentFolder, LPITEMIDLIST pidlParent)
{
	IEnumIDList* pEnum;
	if (FAILED(pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
		return;

	LPITEMIDLIST pidlTemp;
	while (pEnum->Next(1, &pidlTemp, NULL)==S_OK)
	{
		DWORD dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME | SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM;
		pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttribs);

		if (m_OnlyFilesystem)
			if (!(dwAttribs & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)))
				continue;

		SHDESCRIPTIONID did;
		if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &did, sizeof(SHDESCRIPTIONID))))
		{
			const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
			if (did.clsid==LFNE)
				continue;
		}

		InsertItem(pParentFolder, pidlTemp, hParentItem, (dwAttribs & SFGAO_HASSUBFOLDER), p_App->GetShellManager()->ConcatenateItem(pidlParent, pidlTemp));
	}

	pEnum->Release();
}


BEGIN_MESSAGE_MAP(CExplorerTree, CTreeCtrl)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_MESSAGE_MAP()

void CExplorerTree::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	CTreeCtrl::OnDestroy();
}

BOOL CExplorerTree::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CExplorerTree::OnPaint()
{
	if ((m_ExplorerStyle) && (IsCtrlThemed()))
	{
		SetBkColor(0xFFFFFF);
		SetTextColor(0x000000);
	}
	else
		if (IsWindowEnabled())
		{
			SetBkColor(GetSysColor(COLOR_WINDOW));
			SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		}
		else
		{
			SetBkColor(GetSysColor(COLOR_3DFACE));
			SetTextColor(GetSysColor(COLOR_GRAYTEXT));
		}

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	CTreeCtrl::DefWindowProc(WM_PAINT, (WPARAM)dc.m_hDC, NULL);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CExplorerTree::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (m_pContextMenu2)
		return;

	HTREEITEM hItem = NULL;
	if ((point.x==-1) && (point.y==-1))
	{
		hItem = GetSelectedItem();
		if (hItem)
		{
			CRect rectItem;
			if (GetItemRect(hItem, rectItem, FALSE))
			{
				point.x = rectItem.left;
				point.y = rectItem.bottom + 1;
				ClientToScreen(&point);
			}
		}
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		UINT nFlags = 0;
		hItem = HitTest(ptClient, &nFlags);
	}

	if (!hItem)
		return;

	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_STATE;
	tvItem.hItem = hItem;
	if (!GetItem(&tvItem))
		return;

	LPAFX_SHELLITEMINFO pInfo = (LPAFX_SHELLITEMINFO)tvItem.lParam;
	if (!pInfo)
		return;

	IShellFolder* psfFolder = pInfo->pParentFolder;
	if (!psfFolder)
	{
		if (FAILED(SHGetDesktopFolder(&psfFolder)))
			return;
	}
	else
	{
		psfFolder->AddRef();
	}

	if (psfFolder)
	{
		IContextMenu* pcm = NULL;
		if (SUCCEEDED(psfFolder->GetUIObjectOf(GetParent()->GetSafeHwnd(), 1, (LPCITEMIDLIST*)&pInfo->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
		{
			HMENU hPopup = CreatePopupMenu();
			if (hPopup)
				if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, CMF_NORMAL | CMF_EXPLORE)))
				{
					if (tvItem.cChildren)
					{
						CString tmpStr;
						ENSURE(tmpStr.LoadString(LFCommDlgDLL.hResource, tvItem.state & TVIS_EXPANDED ? IDS_COLLAPSE : IDS_EXPAND));
						InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7000, tmpStr);
						InsertMenu(hPopup, 1, MF_BYPOSITION | MF_SEPARATOR, 0x7001, NULL);
						SetMenuDefaultItem(hPopup, 0x7000, 0);
					}

					pcm->QueryInterface(IID_IContextMenu2, (void**)&m_pContextMenu2);
					UINT idCmd = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, GetSafeHwnd(), NULL);
					if (m_pContextMenu2)
					{
						m_pContextMenu2->Release();
						m_pContextMenu2 = NULL;
					}

					if (idCmd==0x7000)
					{
						Expand(hItem, TVE_TOGGLE);
					}
					else
						if (idCmd)
						{
							CWaitCursor wait;

							CMINVOKECOMMANDINFO cmi;
							cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cmi.fMask = 0;
							cmi.hwnd = GetParent()->GetSafeHwnd();
							cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd-1);
							cmi.lpParameters = NULL;
							cmi.lpDirectory = NULL;
							cmi.nShow = SW_SHOWNORMAL;
							cmi.dwHotKey = 0;
							cmi.hIcon = NULL;

							pcm->InvokeCommand(&cmi);
							SetFocus();
						}
				}

			pcm->Release();
		}

		psfFolder->Release();
	}
}

void CExplorerTree::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if (m_TooltipCtrl.IsWindowVisible())
		{
			UINT uFlags;
			HTREEITEM hItem = HitTest(point, &uFlags);
			if ((hItem!=m_HoverItem) || (!(uFlags & TVHT_ONITEM)))
				m_TooltipCtrl.Deactivate();
		}
}

void CExplorerTree::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
}

void CExplorerTree::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		UINT uFlags;
		m_HoverItem = HitTest(point, &uFlags);
		if ((m_HoverItem) && (uFlags & TVHT_ONITEM))
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				TVITEM tvItem;
				ZeroMemory(&tvItem, sizeof(tvItem));
				tvItem.mask = TVIF_PARAM;
				tvItem.hItem = m_HoverItem;
				if (!GetItem(&tvItem))
					return;

				LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)tvItem.lParam;

				HICON hIcon = NULL;
				CSize size(0, 0);
				CString caption;
				CString hint;
				TooltipDataFromPIDL(pItem->pidlFQ, &p_App->m_SystemImageListLarge, hIcon, size, caption, hint);

				ClientToScreen(&point);
				m_TooltipCtrl.Track(point, hIcon, size, caption, hint);
			}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CExplorerTree::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);
	if ((hItem) && (uFlags & TVHT_ONITEM))
	{
		SetFocus();
		SelectItem(hItem);
	}
	else
	{
		CTreeCtrl::OnLButtonDown(nFlags, point);
	}
}

void CExplorerTree::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	SelectItem(HitTest(point, &nFlags));
}

void CExplorerTree::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	switch (pNMTreeView->action)
	{
	case TVE_EXPAND:
		GetChildItems(hItem);
		if (!GetChildItem(hItem))
		{
			TV_ITEM tvItem;
			ZeroMemory(&tvItem, sizeof(tvItem));
			tvItem.hItem = hItem;
			tvItem.mask = TVIF_CHILDREN;
			SetItem(&tvItem);
		}
		break;
	case TVE_COLLAPSE:
		for (HTREEITEM hItemSel = GetSelectedItem(); hItemSel;)
		{
			HTREEITEM hParentItem = GetParentItem(hItemSel);
			if (hParentItem==hItem)
			{
				SelectItem(hItem);
				break;
			}

			hItemSel = hParentItem;
		}

		Expand(hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
		break;
	}

	*pResult = 0;
}

void CExplorerTree::OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)pNMTreeView->itemOld.lParam;

	p_App->GetShellManager()->FreeItem(pItem->pidlFQ);
	p_App->GetShellManager()->FreeItem(pItem->pidlRel);

	if (pItem->pParentFolder)
		pItem->pParentFolder->Release();

	GlobalFree((HGLOBAL)pItem);
	*pResult = 0;
}

LRESULT CExplorerTree::OnShellChange(WPARAM wParam, LPARAM lParam)
{
	LPITEMIDLIST* pidls = (LPITEMIDLIST*)wParam;

	LPITEMIDLIST pidl1FQ = NULL;
	LPITEMIDLIST pidl2FQ = NULL;
	IShellFolder* pDesktop = NULL;
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
	{
		SHGetRealIDL(pDesktop, pidls[0], &pidl1FQ);
		if (pidls[1])
			SHGetRealIDL(pDesktop, pidls[1], &pidl2FQ);
	}

	switch (lParam)
	{
	case SHCNE_DRIVEADD:
	case SHCNE_MEDIAINSERTED:
		{
			if ((m_RootPath==CETR_InternalDrives) || (m_RootPath==CETR_ExternalDrives))
			{
				wchar_t szPath[MAX_PATH];
				if (SUCCEEDED(SHGetPathFromIDList(pidl1FQ, szPath)))
				{
					DWORD DrivesOnSystem = LFGetLogicalDrives(m_RootPath==CETR_InternalDrives ? LFGLD_Internal | LFGLD_Network : LFGLD_External);

					if (DrivesOnSystem & (1 << (szPath[0]-'A')))
					{
						wchar_t szDriveRoot[4];
						wcsncpy_s(szDriveRoot, 4, szPath, 3);

						SHFILEINFO sfi;
						if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES))
							if (sfi.dwAttributes)
								InsertItem(szDriveRoot);
					}
				}
			}
			else
			{
			}

			break;
		}
	case SHCNE_DRIVEREMOVED:
	case SHCNE_MEDIAREMOVED:
	case SHCNE_RMDIR:
		{
			HTREEITEM hItem = FindItem(pDesktop, pidl1FQ);
			if (hItem)
				DeleteItem(hItem);

			break;
		}
	case SHCNE_RENAMEFOLDER:
		{
			HTREEITEM hItem = FindItem(pDesktop, pidl1FQ);

			if (hItem)
			{
				TVITEM tvItem;
				ZeroMemory(&tvItem, sizeof(tvItem));
				tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvItem.hItem = hItem;

				if (GetItem(&tvItem))
				{
					LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)tvItem.lParam;

					IShellFolder* pParentFolder = NULL;
					LPCITEMIDLIST pidlRel = NULL;
					if (SUCCEEDED(SHBindToParent(pidl2FQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
					{
						pItem->pidlFQ = p_App->GetShellManager()->CopyItem(pidl2FQ);
						pItem->pidlRel = p_App->GetShellManager()->CopyItem(pidlRel);
						pItem->pParentFolder = pParentFolder;

						CString strItem = OnGetItemText(pItem);
						tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
						tvItem.iImage = OnGetItemIcon(pItem, FALSE);
						tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);

						SetItem(&tvItem);
					}
				}
			}

			break;
		}
	}

	p_App->GetShellManager()->FreeItem(pidl1FQ);
	if (pidl2FQ)
		p_App->GetShellManager()->FreeItem(pidl2FQ);

	pDesktop->Release();

	return NULL;
}
