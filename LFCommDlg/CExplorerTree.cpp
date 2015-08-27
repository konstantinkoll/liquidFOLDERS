
// CExplorerTree.cpp: Implementierung der Klasse CExplorerTree
//

#include "stdafx.h"
#include "LFCommDlg.h"


INT CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	ExplorerTreeItemData* pItem1 = (ExplorerTreeItemData*)lParam1;
	ExplorerTreeItemData* pItem2 = (ExplorerTreeItemData*)lParam2;

	HRESULT hResult = ((IShellFolder*)lParamSort)->CompareIDs(0, pItem1->pidlRel, pItem2->pidlRel);
	return FAILED(hResult) ? 0 : (SHORT)SCODE_CODE(GetScode(hResult));
}


// CExplorerTree
//

CExplorerTree::CExplorerTree()
	: CTreeCtrl()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CExplorerTree";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CExplorerTree", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_pContextMenu2 = NULL;
	m_Hover = FALSE;
	m_HoverItem = NULL;
	m_ExplorerStyle = FALSE;
	m_SHChangeNotifyRegister = NULL;
}

void CExplorerTree::PreSubclassWindow()
{
	// Icons
	SetImageList(&LFGetApp()->m_SystemImageListSmall, 0);

	// Schrift
	LOGFONT lf;
	LFGetApp()->m_DefaultFont.GetLogFont(&lf);
	SetItemHeight((SHORT)(max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON))+(LFGetApp()->OSVersion<OS_Vista ? 2 : 6)));
	SetFont(&LFGetApp()->m_DefaultFont, FALSE);

	// Benachrichtigung, wenn sich Items ändern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_SHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_ShellLevel | SHCNRF_NewDelivery,
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED |
			SHCNE_MKDIR | SHCNE_RMDIR | SHCNE_RENAMEFOLDER | SHCNE_UPDATEITEM | SHCNE_INTERRUPT,
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

	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;
	return pItem->pidlFQ;
}

BOOL CExplorerTree::GetSelectedPath(LPWSTR Path)
{
	LPITEMIDLIST pidl = GetSelectedPIDL();
	return pidl ? SHGetPathFromIDList(pidl, Path) : FALSE;
}

LRESULT CExplorerTree::WindowProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITMENUPOPUP:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (m_pContextMenu2)
		{
			m_pContextMenu2->HandleMenuMsg(Message, wParam, lParam);
			return 0;
		}
	}

	return CTreeCtrl::WindowProc(Message, wParam, lParam);
}

BOOL CExplorerTree::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if ((pMsg->wParam==VK_RETURN) || (pMsg->wParam==VK_ESCAPE))
		{
			CEdit* pEditCtrl = GetEditControl();
			if (pEditCtrl)
			{
				pEditCtrl->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);

				return TRUE;
			}
		}

		break;

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
	case WM_MOUSEHWHEEL:
		LFGetApp()->HideTooltip();
		break;
	}

	return CTreeCtrl::PreTranslateMessage(pMsg);
}

CString CExplorerTree::OnGetItemText(ExplorerTreeItemData* pItem)
{
	ASSERT(pItem);

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME)))
		return sfi.szDisplayName;

	return _T("?");
}

INT CExplorerTree::OnGetItemIcon(ExplorerTreeItemData* pItem, BOOL bSelected)
{
	ASSERT(pItem);

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | (bSelected ? SHGFI_OPENICON : SHGFI_LINKOVERLAY))))
		return sfi.iIcon;

	return -1;
}

HTREEITEM CExplorerTree::InsertItem(LPITEMIDLIST pidlFQ, LPITEMIDLIST pidlRel, ULONG dwAttributes, HTREEITEM hParent)
{
	if (!pidlRel)
		return NULL;

	TV_ITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
	pItem->pidlRel = pidlRel;
	pItem->pidlFQ = pidlFQ;
	pItem->dwAttributes = dwAttributes;
	tvItem.lParam = (LPARAM)pItem;

	CString strItem = OnGetItemText(pItem);
	tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
	tvItem.iImage = OnGetItemIcon(pItem, FALSE);
	tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);
	tvItem.cChildren = (dwAttributes & SFGAO_HASSUBFOLDER);

	TV_INSERTSTRUCT tvInsert;
	tvInsert.item = tvItem;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.hParent = hParent;

	return CTreeCtrl::InsertItem(&tvInsert);
}

HTREEITEM CExplorerTree::InsertItem(WCHAR* Path, HTREEITEM hParent)
{
	const SFGAOF dwAttributes = SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_HASPROPSHEET | SFGAO_CANRENAME | SFGAO_CANDELETE;

	LPITEMIDLIST pidlFQ;
	if (FAILED(SHParseDisplayName(Path, NULL, &pidlFQ, dwAttributes, NULL)))
		return NULL;

	IShellFolder* pParentFolder = NULL;
	LPCITEMIDLIST pidlRel = NULL;
	if (FAILED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
		return NULL;

	pParentFolder->Release();

	return InsertItem(pidlFQ, LFGetApp()->GetShellManager()->CopyItem(pidlRel), dwAttributes, hParent);
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

		hItem = InsertItem(pidl, LFGetApp()->GetShellManager()->CopyItem(pidl));
	}
	else
	{
		hItem = InsertItem(m_RootPath.GetBuffer());
		Select(hItem, TVGN_CARET);
	}

	if ((hItem) && (!(GetStyle() & TVS_LINESATROOT)))
		Expand(hItem, TVE_EXPAND);
}

BOOL CExplorerTree::ChildrenContainPath(HTREEITEM hParentItem, LPWSTR Path)
{
	HTREEITEM hItem = GetChildItem(hParentItem);

	while (hItem)
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM;
		tvItem.hItem = hItem;

		if (GetItem(&tvItem))
		{
			ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;

			WCHAR tmpPath[MAX_PATH];
			if (SHGetPathFromIDList(pItem->pidlFQ, tmpPath))
				if (wcscmp(tmpPath, Path)==0)
					return TRUE;
		}

		hItem = GetNextSiblingItem(hItem);
	}

	return FALSE;
}

BOOL CExplorerTree::DeletePath(LPWSTR Path)
{
	BOOL Deleted = FALSE;

	CList<HTREEITEM> lstItems;
	HTREEITEM hItem = GetRootItem();
	while (hItem)
	{
		lstItems.AddTail(hItem);
		hItem = GetNextSiblingItem(hItem);
	}

	hItem = lstItems.IsEmpty() ? NULL : lstItems.RemoveHead();
	while (hItem)
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
		tvItem.hItem = hItem;

		if (GetItem(&tvItem))
		{
			ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;
			BOOL AddChildren = TRUE;

			WCHAR tmpPath[MAX_PATH];
			if (SHGetPathFromIDList(pItem->pidlFQ, tmpPath))
				if (wcscmp(tmpPath, Path)==0)
				{
					HTREEITEM hParentItem = GetParentItem(hItem);

					DeleteItem(hItem);

					if (hParentItem)
						if (!GetChildItem(hParentItem))
						{
							TVITEM tvParentItem;
							ZeroMemory(&tvParentItem, sizeof(tvParentItem));
							tvParentItem.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_STATE;
							tvParentItem.hItem = hParentItem;
							tvParentItem.stateMask = TVIS_EXPANDED;

							if (GetItem(&tvParentItem))
							{
								tvParentItem.cChildren = FALSE;
								tvParentItem.state &= !TVIS_EXPANDED;

								SetItem(&tvParentItem);
							}
						}

					AddChildren = FALSE;
					Deleted = TRUE;
				}

			if (AddChildren)
			{
				hItem = GetChildItem(hItem);
				while (hItem)
				{
					lstItems.AddTail(hItem);
					hItem = GetNextSiblingItem(hItem);
				}
			}
		}

		hItem = lstItems.IsEmpty() ? NULL : lstItems.RemoveHead();
	}

	return Deleted;
}

BOOL CExplorerTree::AddPath(LPWSTR Path, LPWSTR Parent)
{
	BOOL Added = FALSE;

	CList<HTREEITEM> lstItems;
	HTREEITEM hItem = GetRootItem();
	while (hItem)
	{
		lstItems.AddTail(hItem);
		hItem = GetNextSiblingItem(hItem);
	}

	hItem = lstItems.IsEmpty() ? NULL : lstItems.RemoveHead();
	while (hItem)
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM | TVIF_CHILDREN;
		tvItem.hItem = hItem;

		if (GetItem(&tvItem))
		{
			ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;
			BOOL AddChildren = TRUE;

			WCHAR tmpPath[MAX_PATH];
			if (SHGetPathFromIDList(pItem->pidlFQ, tmpPath))
				if (wcscmp(tmpPath, Parent)==0)
				{
					tvItem.cChildren = TRUE;
					SetItem(&tvItem);

					if (GetItemState(hItem, TVIS_EXPANDED))
						if (!ChildrenContainPath(hItem, Path))
						{
							InsertItem(Path, hItem);
							Added = TRUE;
						}

					AddChildren = FALSE;
				}

			if (AddChildren)
			{
				hItem = GetChildItem(hItem);
				while (hItem)
				{
					lstItems.AddTail(hItem);
					hItem = GetNextSiblingItem(hItem);
				}
			}
		}

		hItem = lstItems.IsEmpty() ? NULL : lstItems.RemoveHead();
	}

	return Added;
}

void CExplorerTree::UpdateChildPIDLs(HTREEITEM hParentItem, LPITEMIDLIST pidlParent)
{
	HTREEITEM hItem = GetChildItem(hParentItem);

	while (hItem)
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM;
		tvItem.hItem = hItem;

		if (GetItem(&tvItem))
		{
			ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;

			LFGetApp()->GetShellManager()->FreeItem(pItem->pidlFQ);
			pItem->pidlFQ = LFGetApp()->GetShellManager()->ConcatenateItem(pidlParent, pItem->pidlRel);

			UpdateChildPIDLs(hItem, pItem->pidlFQ);
		}

		hItem = GetNextSiblingItem(hItem);
	}
}

void CExplorerTree::UpdatePath(LPWSTR Path1, LPWSTR Path2)
{
	CList<HTREEITEM> lstItems;
	HTREEITEM hItem = GetRootItem();
	while (hItem)
	{
		lstItems.AddTail(hItem);
		hItem = GetNextSiblingItem(hItem);
	}

	hItem = lstItems.IsEmpty() ? NULL : lstItems.RemoveHead();
	while (hItem)
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvItem.hItem = hItem;

		if (GetItem(&tvItem))
		{
			ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;

			WCHAR tmpPath[MAX_PATH];
			if (SHGetPathFromIDList(pItem->pidlFQ, tmpPath))
				if (wcscmp(tmpPath, Path1)==0)
				{
					const SFGAOF dwAttributes = SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_HASPROPSHEET | SFGAO_CANRENAME | SFGAO_CANDELETE;
					LPITEMIDLIST pidlFQ;
					if (SUCCEEDED(SHParseDisplayName(Path2, NULL, &pidlFQ, dwAttributes, NULL)))
					{
						IShellFolder* pParentFolder = NULL;
						LPCITEMIDLIST pidlRel = NULL;
						if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
						{
							LFGetApp()->GetShellManager()->FreeItem(pItem->pidlFQ);
							LFGetApp()->GetShellManager()->FreeItem(pItem->pidlRel);

							pItem->pidlFQ = pidlFQ;
							pItem->pidlRel = LFGetApp()->GetShellManager()->CopyItem(pidlRel);
							pItem->dwAttributes = dwAttributes;

							CString strItem = OnGetItemText(pItem);
							tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
							tvItem.iImage = OnGetItemIcon(pItem, FALSE);
							tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);

							SetItem(&tvItem);
							UpdateChildPIDLs(hItem, pidlFQ);

							pParentFolder->Release();
						}
					}
				}

			hItem = GetChildItem(hItem);
			while (hItem)
			{
				lstItems.AddTail(hItem);
				hItem = GetNextSiblingItem(hItem);
			}
		}

		hItem = lstItems.IsEmpty() ? NULL : lstItems.RemoveHead();
	}
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
	CWaitCursor csr;

	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hParentItem;
	if (!GetItem(&tvItem))
		return FALSE;

	SetRedraw(FALSE);

	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;
	EnumObjects(hParentItem, pItem->pidlFQ);

	SetRedraw(TRUE);
	RedrawWindow();

	return TRUE;
}

void CExplorerTree::EnumObjects(HTREEITEM hParentItem, LPITEMIDLIST pidlParent)
{
	IShellFolder* pDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return;

	IShellFolder* pParentFolder = NULL;
	if (LFGetApp()->GetShellManager()->GetItemSize(pidlParent)==2)
	{
		pParentFolder = pDesktop;
		pDesktop->AddRef();
	}
	else
		if (FAILED(pDesktop->BindToObject(pidlParent, NULL, IID_IShellFolder, (void**)&pParentFolder)))
		{
			pDesktop->Release();
			return;
		}

	IEnumIDList* pEnum;
	if (SUCCEEDED(pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
		if(pEnum)
		{
			LPITEMIDLIST pidlTemp;
			while (pEnum->Next(1, &pidlTemp, NULL)==S_OK)
			{
				DWORD dwAttributes = SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_HASPROPSHEET | SFGAO_CANRENAME | SFGAO_CANDELETE;
				pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttributes);

				// Don't include virtual branches
				if (!(dwAttributes & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)))
					continue;

				// Don't include liquidFOLDERS
				SHDESCRIPTIONID did;
				if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &did, sizeof(SHDESCRIPTIONID))))
				{
					const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
					if (did.clsid==LFNE)
						continue;
				}

				// Don't include file junctions
				LPITEMIDLIST pidlFQ = LFGetApp()->GetShellManager()->ConcatenateItem(pidlParent, pidlTemp);

				WCHAR Path[MAX_PATH];
				if (SUCCEEDED(SHGetPathFromIDList(pidlFQ, Path)))
				{
					DWORD Attr = GetFileAttributes(Path);
					if ((Attr!=INVALID_FILE_ATTRIBUTES) && (!(Attr & FILE_ATTRIBUTE_DIRECTORY)))
					{
						LFGetApp()->GetShellManager()->FreeItem(pidlFQ);
						continue;
					}
				}

				InsertItem(pidlFQ, pidlTemp, dwAttributes, hParentItem);
			}

			pEnum->Release();
		}

	TV_SORTCB tvSort;
	tvSort.hParent = hParentItem;
	tvSort.lpfnCompare = CompareProc;
	tvSort.lParam = (LPARAM)pParentFolder;
	SortChildrenCB(&tvSort);

	pParentFolder->Release();
	pDesktop->Release();
}


BEGIN_MESSAGE_MAP(CExplorerTree, CTreeCtrl)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_MESSAGE_MAP()

void CExplorerTree::OnDestroy()
{
	if (m_SHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_SHChangeNotifyRegister));

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

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	CTreeCtrl::DefWindowProc(WM_PAINT, (WPARAM)dc.m_hDC, NULL);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CExplorerTree::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (m_pContextMenu2)
		return;

	HTREEITEM hItem = NULL;
	if ((point.x<0) && (point.y<0))
	{
		hItem = GetSelectedItem();
		if (!hItem)
			return;

		CRect rectItem;
		if (GetItemRect(hItem, rectItem, FALSE))
		{
			point.x = rectItem.left;
			point.y = rectItem.bottom+1;
			ClientToScreen(&point);
		}
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		UINT nFlags;
		hItem = HitTest(ptClient, &nFlags);
		if ((!hItem) || (!(nFlags & TVHT_ONITEM)))
			return;
	}

	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_STATE;
	tvItem.hItem = hItem;
	if (!GetItem(&tvItem))
		return;

	ExplorerTreeItemData* pInfo = (ExplorerTreeItemData*)tvItem.lParam;
	if (!pInfo)
		return;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(pInfo->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetParent()->GetSafeHwnd(), 1, (LPCITEMIDLIST*)&pInfo->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_CANRENAME;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				if (tvItem.cChildren)
				{
					CString tmpStr((LPCSTR)(tvItem.state & TVIS_EXPANDED ? IDS_COLLAPSE : IDS_EXPAND));

					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7000, tmpStr);
					InsertMenu(hPopup, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
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
						CHAR Verb[256] = "";
						pcm->GetCommandString(idCmd-1, GCS_VERBA, NULL, Verb, 256);

						if (strcmp(Verb, "rename")==0)
						{
							EditLabel(hItem);
						}
						else
						{
							CWaitCursor csr;

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
			}
		}

		pcm->Release();
	}

	pParentFolder->Release();
}

void CExplorerTree::OnMouseMove(UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	if ((nFlags & MK_RBUTTON) && (hItem) && (uFlags & TVHT_ONITEM))
	{
		SetFocus();
		SelectItem(hItem);
	}

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((LFGetApp()->IsTooltipVisible()) && ((hItem!=m_HoverItem) || (!(uFlags & TVHT_ONITEM))))
			LFGetApp()->HideTooltip();

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CExplorerTree::OnMouseLeave()
{
	LFGetApp()->HideTooltip();
	m_Hover = FALSE;

	CTreeCtrl::OnMouseLeave();
}

void CExplorerTree::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		UINT uFlags;
		m_HoverItem = HitTest(point, &uFlags);
		if ((m_HoverItem) && (uFlags & TVHT_ONITEM) && (!GetEditControl()))
			if (!LFGetApp()->IsTooltipVisible())
			{
				TVITEM tvItem;
				ZeroMemory(&tvItem, sizeof(tvItem));
				tvItem.mask = TVIF_PARAM;
				tvItem.hItem = m_HoverItem;
				if (!GetItem(&tvItem))
					return;

				ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)tvItem.lParam;

				HICON hIcon = NULL;
				CString Caption;
				CString Hint;
				TooltipDataFromPIDL(pItem->pidlFQ, &LFGetApp()->m_SystemImageListExtraLarge, hIcon, Caption, Hint);

				LFGetApp()->ShowTooltip(this, point, Caption, Hint, hIcon);
			}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CExplorerTree::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);
	if ((hItem) && (uFlags & TVHT_ONITEM))
		SelectItem(hItem);
}

void CExplorerTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar==VK_F2) && (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
	{
		EditLabel(GetSelectedItem());
		return;
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
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
	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->itemOld.lParam;

	LFGetApp()->GetShellManager()->FreeItem(pItem->pidlFQ);
	LFGetApp()->GetShellManager()->FreeItem(pItem->pidlRel);

	GlobalFree((HGLOBAL)pItem);
	*pResult = 0;
}

void CExplorerTree::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO* pNMTreeView = (NMTVDISPINFO*)pNMHDR;
	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->item.lParam;

	CEdit* edit = GetEditControl();
	if (edit)
	{
		IShellFolder* pParentFolder = NULL;
		if (SUCCEEDED(SHBindToParent(pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		{
			STRRET strret;
			if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pItem->pidlRel, SHGDN_FOREDITING, &strret)))
				if (strret.uType==STRRET_WSTR)
				{
					edit->SetWindowText(strret.pOleStr);
					edit->SetSel(0, -1);
				}

			pParentFolder->Release();
		}
	}

	*pResult = !(pItem->dwAttributes & SFGAO_CANRENAME);
}

void CExplorerTree::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO* pNMTreeView = (NMTVDISPINFO*)pNMHDR;
	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->item.lParam;

	*pResult = FALSE;

	CEdit* pWndEdit = GetEditControl();
	if (pWndEdit)
	{
		CString Name;
		pWndEdit->GetWindowText(Name);

		if (!Name.IsEmpty())
		{
			IShellFolder* pParentFolder = NULL;
			if (SUCCEEDED(SHBindToParent(pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
			{
				LPITEMIDLIST pidlRel = NULL;
				if (SUCCEEDED(pParentFolder->SetNameOf(m_hWnd, pItem->pidlRel, Name, SHGDN_NORMAL, &pidlRel)))
				{
					LPITEMIDLIST pidlParent = NULL;
					LFGetApp()->GetShellManager()->GetParentItem(pItem->pidlFQ, pidlParent);

					LFGetApp()->GetShellManager()->FreeItem(pItem->pidlFQ);
					LFGetApp()->GetShellManager()->FreeItem(pItem->pidlRel);

					pItem->pidlFQ = LFGetApp()->GetShellManager()->ConcatenateItem(pidlParent, pidlRel);
					pItem->pidlRel = pidlRel;

					UpdateChildPIDLs(pNMTreeView->item.hItem, pItem->pidlFQ);
					LFGetApp()->GetShellManager()->FreeItem(pidlParent);

					*pResult = TRUE;
				}

				pParentFolder->Release();
			}
		}
	}

	if (*pResult)
	{
		m_strBuffer = OnGetItemText(pItem);
		pNMTreeView->item.pszText = m_strBuffer.GetBuffer(m_strBuffer.GetLength());
	}
}

LRESULT CExplorerTree::OnShellChange(WPARAM wParam, LPARAM lParam)
{
	PIDLIST_ABSOLUTE* pidls;
	LONG Event;
	HANDLE hNotifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &pidls, &Event);

	if (!hNotifyLock)
		return NULL;

	WCHAR Path1[MAX_PATH] = L"";
	WCHAR Path2[MAX_PATH] = L"";
	WCHAR Parent1[MAX_PATH] = L"";
	WCHAR Parent2[MAX_PATH] = L"";

	SHGetPathFromIDList(pidls[0], Path1);

	wcscpy_s(Parent1, MAX_PATH, Path1);
	WCHAR* Ptr = wcsrchr(Parent1, L'\\');
	if (Ptr<=&Parent1[2])
		Ptr = &Parent1[3];
	*Ptr = '\0';

	if (pidls[1])
	{
		SHGetPathFromIDList(pidls[1], Path2);

		wcscpy_s(Parent2, MAX_PATH, Path2);
		Ptr = wcsrchr(Parent2, L'\\');
		if (Ptr<=&Parent2[2])
			Ptr = &Parent2[3];
		*Ptr = '\0';
	}

	BOOL NotifyOwner = FALSE;

	switch (Event)
	{
	case SHCNE_MKDIR:
		if ((Path1[0]!='\0') && (Parent1[0]!='\0') && (wcscmp(Path1, Parent1)!=0))
			if (AddPath(Path1, Parent1))
				NotifyOwner = TRUE;

		break;

	case SHCNE_DRIVEREMOVED:
	case SHCNE_MEDIAREMOVED:
	case SHCNE_RMDIR:
		if (Path1[0]!='\0')
			if (DeletePath(Path1))
				NotifyOwner = TRUE;

		break;

	case SHCNE_RENAMEFOLDER:
		if ((Path1[0]!='\0') && (Path2[0]!='\0'))
			if (wcscmp(Parent1, Parent2)==0)
			{
				UpdatePath(Path1, Path2);
			}
			else
			{
				if (DeletePath(Path1))
					NotifyOwner = TRUE;
				if ((Parent2[0]!='\0') && (wcscmp(Path2, Parent2)!=0))
					if (AddPath(Path2, Parent2))
						NotifyOwner = TRUE;
			}

		break;

	case SHCNE_UPDATEITEM:
		wcscpy_s(Path2, MAX_PATH, Parent1);
		wcscat_s(Path2, MAX_PATH, L"\\desktop.ini");
		if (wcscmp(Path1, Path2)==0)
			UpdatePath(Parent1, Parent1);

		break;
	}

	SHChangeNotification_Unlock(hNotifyLock);

	if (NotifyOwner)
	{
		NMTREEVIEW tag;
		ZeroMemory(&tag, sizeof(tag));
		tag.hdr.hwndFrom = m_hWnd;
		tag.hdr.idFrom = GetDlgCtrlID();
		tag.hdr.code = TVN_SELCHANGED;

		tag.itemNew.hItem = GetSelectedItem();
		if (tag.itemNew.hItem)
		{
			tag.itemNew.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
			GetItem(&tag.itemNew);
		}

		GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
	}

	return NULL;
}
