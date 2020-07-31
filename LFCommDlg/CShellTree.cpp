
// CShellTree.cpp: Implementierung der Klasse CShellTree
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


// CShellTree
//

#define SFGAO_REQUIRED     (SFGAO_HASSUBFOLDER | SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_HASPROPSHEET | SFGAO_CANRENAME | SFGAO_CANDELETE)

CShellTree::CShellTree()
	: CTreeCtrl()
{
	CONSTRUCTOR_TOOLTIP();

	m_pContextMenu2 = NULL;
	m_SHChangeNotifyRegister = NULL;
}

BOOL CShellTree::Create(CWnd* pParentWnd, UINT nID)
{
	return CTreeCtrl::Create(TVS_HASBUTTONS | TVS_EDITLABELS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_NOTOOLTIPS | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

LPCITEMIDLIST CShellTree::GetSelectedPIDL() const
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

	return ((ExplorerTreeItemData*)tvItem.lParam)->pidlFQ;
}

BOOL CShellTree::GetSelectedPath(LPWSTR Path) const
{
	ASSERT(Path);

	LPCITEMIDLIST pidl = GetSelectedPIDL();

	return pidl ? SHGetPathFromIDList(pidl, Path) : FALSE;
}

LRESULT CShellTree::WindowProc(UINT Message, WPARAM wParam, LPARAM lParam)
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

		break;
/*
	case WM_KEYDOWN:
		if ((wParam==VK_RETURN) || (wParam==VK_ESCAPE))
		{
			CEdit* pWndEdit = GetEditControl();
			if (pWndEdit)
			{
				pWndEdit->SendMessage(WM_KEYDOWN, wParam, lParam);

				return TRUE;
			}
		}

		break;*/
	}

	return CTreeCtrl::WindowProc(Message, wParam, lParam);
}

CString CShellTree::GetItemText(LPITEMIDLIST pidlFQ) const
{
	ASSERT(pidlFQ);

	SHFILEINFO ShellFileInfo;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &ShellFileInfo, sizeof(ShellFileInfo), SHGFI_PIDL | SHGFI_DISPLAYNAME)))
		return ShellFileInfo.szDisplayName;

	return _T("?");
}

INT CShellTree::GetItemIcon(LPITEMIDLIST pidlFQ, BOOL bSelected) const
{
	ASSERT(pidlFQ);

	SHFILEINFO ShellFileInfo;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &ShellFileInfo, sizeof(ShellFileInfo), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | (bSelected ? SHGFI_OPENICON : SHGFI_LINKOVERLAY))))
		return ShellFileInfo.iIcon;

	return -1;
}

HTREEITEM CShellTree::InsertItem(LPITEMIDLIST pidlFQ, LPITEMIDLIST pidlRel, SFGAOF dwAttributes, HTREEITEM hParent)
{
	if (!pidlRel)
		return NULL;

	TV_ITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;

	ExplorerTreeItemData* pItem = new ExplorerTreeItemData;
	pItem->pidlRel = pidlRel;
	pItem->pidlFQ = pidlFQ;
	pItem->dwAttributes = dwAttributes;
	tvItem.lParam = (LPARAM)pItem;

	CString strItem = GetItemText(pItem);
	tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
	tvItem.iImage = GetItemIcon(pItem, FALSE);
	tvItem.iSelectedImage = GetItemIcon(pItem, TRUE);
	tvItem.cChildren = (dwAttributes & SFGAO_HASSUBFOLDER);

	TV_INSERTSTRUCT tvInsert;
	tvInsert.item = tvItem;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.hParent = hParent;

	return CTreeCtrl::InsertItem(&tvInsert);
}

HTREEITEM CShellTree::InsertItem(LPCWSTR Path, HTREEITEM hParent)
{
	SFGAOF dwAttributes;
	LPITEMIDLIST pidlFQ;
	if (FAILED(SHParseDisplayName(Path, NULL, &pidlFQ, SFGAO_REQUIRED, &dwAttributes)))
		return NULL;

	IShellFolder* pParentFolder;
	LPCITEMIDLIST pidlRel;
	if (FAILED(SHBindToParent(pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, &pidlRel)))
	{
		LFGetApp()->GetShellManager()->FreeItem(pidlFQ);

		return NULL;
	}

	pParentFolder->Release();

	return InsertItem(pidlFQ, LFGetApp()->GetShellManager()->CopyItem(pidlRel), dwAttributes, hParent);
}

BOOL CShellTree::ChildrenContainPath(HTREEITEM hParentItem, LPCWSTR Path) const
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

BOOL CShellTree::DeletePath(LPCWSTR Path)
{
	ASSERT(Path);

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

BOOL CShellTree::AddPath(LPCWSTR Path, LPCWSTR Parent)
{
	ASSERT(Path);
	ASSERT(Parent);

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

void CShellTree::UpdateChildPIDLs(HTREEITEM hParentItem, LPITEMIDLIST pidlParent)
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

void CShellTree::UpdatePath(LPCWSTR Path1, LPCWSTR Path2)
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
					SFGAOF dwAttributes;
					LPITEMIDLIST pidlFQ;
					if (SUCCEEDED(SHParseDisplayName(Path2, NULL, &pidlFQ, SFGAO_REQUIRED, &dwAttributes)))
					{
						IShellFolder* pParentFolder;
						LPCITEMIDLIST pidlRel;
						if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, &pidlRel)))
						{
							LFGetApp()->GetShellManager()->FreeItem(pItem->pidlFQ);
							LFGetApp()->GetShellManager()->FreeItem(pItem->pidlRel);

							pItem->pidlFQ = pidlFQ;
							pItem->pidlRel = LFGetApp()->GetShellManager()->CopyItem(pidlRel);
							pItem->dwAttributes = dwAttributes;

							CString strItem = GetItemText(pItem);
							tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
							tvItem.iImage = GetItemIcon(pItem, FALSE);
							tvItem.iSelectedImage = GetItemIcon(pItem, TRUE);

							SetItem(&tvItem);
							UpdateChildPIDLs(hItem, pidlFQ);

							pParentFolder->Release();
						}
						else
						{
							LFGetApp()->GetShellManager()->FreeItem(pidlFQ);
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

BOOL CShellTree::GetChildItems(HTREEITEM hParentItem)
{
	CWaitCursor WaitCursor;

	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hParentItem;
	if (!GetItem(&tvItem))
		return FALSE;

	SetRedraw(FALSE);

	EnumObjects(hParentItem, ((ExplorerTreeItemData*)tvItem.lParam)->pidlFQ);

	SetRedraw(TRUE);
	RedrawWindow();

	return TRUE;
}

void CShellTree::EnumObjects(HTREEITEM hParentItem, LPITEMIDLIST pidlParent)
{
	IShellFolder* pDesktop;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return;

	IShellFolder* pParentFolder;
	if (LFGetApp()->GetShellManager()->GetItemSize(pidlParent)==2)
	{
		pParentFolder = pDesktop;
		pDesktop->AddRef();
	}
	else
		if (FAILED(pDesktop->BindToObject(pidlParent, NULL, IID_IShellFolder, (LPVOID*)&pParentFolder)))
		{
			pDesktop->Release();
			return;
		}

	IEnumIDList* pEnum;
	if (SUCCEEDED(pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
		if (pEnum)
		{
			LPITEMIDLIST pidlTemp;
			while (pEnum->Next(1, &pidlTemp, NULL)==S_OK)
			{
				SFGAOF dwAttributes = SFGAO_REQUIRED;
				if (FAILED(pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttributes)))
					continue;

				// Don't include virtual branches
				if (!(dwAttributes & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)))
					continue;

				// Don't include liquidFOLDERS Explorer namespace extension
				/*SHDESCRIPTIONID did;
				if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &did, sizeof(SHDESCRIPTIONID))))
				{
					const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
					if (did.clsid==LFNE)
						continue;
				}*/

				// Don't include file junctions
				LPITEMIDLIST pidlFQ = LFGetApp()->GetShellManager()->ConcatenateItem(pidlParent, pidlTemp);

				WCHAR Path[MAX_PATH];
				if (SUCCEEDED(SHGetPathFromIDList(pidlFQ, Path)))
				{
					DWORD Attr = GetFileAttributes(Path);
					if ((Attr!=INVALID_FILE_ATTRIBUTES) && !(Attr & FILE_ATTRIBUTE_DIRECTORY))
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

INT CShellTree::ItemAtPosition(CPoint /*point*/) const
{
	return -1;
}

CPoint CShellTree::PointAtPosition(CPoint /*point*/) const
{
	return CPoint(-1, -1);
}

LPCVOID CShellTree::PtrAtPosition(CPoint point) const
{
	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);

	return (hItem && (uFlags & TVHT_ONITEM)) ? hItem : NULL;
}

void CShellTree::InvalidateItem(INT /*Index*/)
{
	Invalidate();
}

void CShellTree::InvalidatePoint(const CPoint& /*point*/)
{
	Invalidate();
}

void CShellTree::InvalidatePtr(LPCVOID /*Ptr*/)
{
	Invalidate();
}

void CShellTree::ShowTooltip(const CPoint& point)
{
	if (!GetEditControl())
	{
		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM;
		tvItem.hItem = (HTREEITEM)m_HoverPtr;

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


#define ADDITIONALCODE \
	case WM_KEYDOWN: \
		{ \
			CEdit* pWndEdit = GetEditControl(); \
			if (pWndEdit && ((pMsg->wParam==VK_RETURN) || (pMsg->wParam==VK_ESCAPE))) \
			{ \
				pWndEdit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam); \
				return TRUE; \
			} \
			break; \
		} \

IMPLEMENT_TOOLTIP_WHEELWITHADDITIONALCODE(CShellTree, CTreeCtrl, ADDITIONALCODE)

BEGIN_TOOLTIP_MAP(CShellTree, CTreeCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_TOOLTIP_MAP()

INT CShellTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Theme
	if (LFGetApp()->m_ThemeLibLoaded && (LFGetApp()->OSVersion>=OS_Vista))
	{
		LFGetApp()->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);

		ModifyStyle(0, TVS_TRACKSELECT);
	}

	// Font
	SetFont(&LFGetApp()->m_DefaultFont, FALSE);

	// Icons
	SetImageList(&LFGetApp()->m_SystemImageListSmall, 0);

	// Shell notifications
	const SHChangeNotifyEntry shCNE = { NULL, TRUE };

	m_SHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_ShellLevel | SHCNRF_NewDelivery,
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED |
			SHCNE_MKDIR | SHCNE_RMDIR | SHCNE_RENAMEFOLDER | SHCNE_UPDATEITEM | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);

	// Populate tree
	LPITEMIDLIST pidl;
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl)))
	{
		HTREEITEM hItem;
		if ((hItem=InsertItem(pidl, LFGetApp()->GetShellManager()->CopyItem(pidl)))!=NULL)
			Expand(hItem, TVE_EXPAND);
	}

	return 0;
}

void CShellTree::OnDestroy()
{
	if (m_SHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_SHChangeNotifyRegister));

	CTreeCtrl::OnDestroy();
}

LRESULT CShellTree::OnNcHitTest(CPoint point)
{
	CRect rectWindow;
	GetAncestor(GA_ROOT)->GetWindowRect(rectWindow);

	if (!rectWindow.PtInRect(point))
		return HTNOWHERE;

	if ((point.x<rectWindow.left+BACKSTAGEGRIPPER) || (point.x>=rectWindow.right-BACKSTAGEGRIPPER) ||
		(point.y<rectWindow.top+BACKSTAGEGRIPPER) || (point.y>=rectWindow.bottom-BACKSTAGEGRIPPER))
		return HTTRANSPARENT;

	return CTreeCtrl::OnNcHitTest(point);
}

BOOL CShellTree::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CShellTree::OnPaint()
{
	if (IsCtrlThemed())
	{
		SetBkColor(0xFFFFFF);
		SetTextColor(0x000000);
	}
	else
	{
		SetBkColor(GetSysColor(COLOR_WINDOW));
		SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
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

void CShellTree::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ((pWnd!=this) || m_pContextMenu2)
		return;

	HTREEITEM hItem = NULL;
	if ((point.x<0) && (point.y<0))
	{
		if ((hItem=GetSelectedItem())==NULL)
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
		CPoint pt(point);
		ScreenToClient(&pt);

		UINT nFlags;
		hItem = HitTest(pt, &nFlags);
		if (!hItem || (!(nFlags & TVHT_ONITEM)))
			return;

		SelectItem(hItem);
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

	IShellFolder* pParentFolder;
	if (FAILED(SHBindToParent(pInfo->pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, NULL)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetParent()->GetSafeHwnd(), 1, (LPCITEMIDLIST*)&pInfo->pidlRel, IID_IContextMenu, NULL, (LPVOID*)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_CANRENAME;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				if (tvItem.cChildren)
				{
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7000, CString((LPCSTR)(tvItem.state & TVIS_EXPANDED ? IDS_COLLAPSE : IDS_EXPAND)));
					InsertMenu(hPopup, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

					SetMenuDefaultItem(hPopup, 0x7000, 0);
				}

				pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&m_pContextMenu2);
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
						CHAR Verb[256];
						Verb[0] = '\0';

						pcm->GetCommandString(idCmd-1, GCS_VERBA, NULL, Verb, 256);

						if (strcmp(Verb, "rename")==0)
						{
							EditLabel(hItem);
						}
						else
						{
							CWaitCursor WaitCursor;

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

void CShellTree::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	UINT uFlags;
	HTREEITEM hItem = HitTest(point, &uFlags);
	if (hItem && (uFlags & TVHT_ONITEM))
		SelectItem(hItem);
}

void CShellTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar==VK_F2) && (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
	{
		EditLabel(GetSelectedItem());
		return;
	}

	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CShellTree::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
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

void CShellTree::OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->itemOld.lParam;

	LFGetApp()->GetShellManager()->FreeItem(pItem->pidlFQ);
	LFGetApp()->GetShellManager()->FreeItem(pItem->pidlRel);

	delete pItem;
	*pResult = 0;
}

void CShellTree::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO* pNMTreeView = (NMTVDISPINFO*)pNMHDR;
	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->item.lParam;

	CEdit* pWndEdit = GetEditControl();
	if (pWndEdit)
	{
		IShellFolder* pParentFolder;
		if (SUCCEEDED(SHBindToParent(pItem->pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, NULL)))
		{
			STRRET strret;
			if (SUCCEEDED(pParentFolder->GetDisplayNameOf(pItem->pidlRel, SHGDN_FOREDITING, &strret)))
				if (strret.uType==STRRET_WSTR)
				{
					pWndEdit->SetWindowText(strret.pOleStr);
					pWndEdit->SetSel(0, -1);
				}

			pParentFolder->Release();
		}
	}

	*pResult = !(pItem->dwAttributes & SFGAO_CANRENAME);
}

void CShellTree::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO* pNMTreeView = (NMTVDISPINFO*)pNMHDR;
	ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->item.lParam;

	*pResult = FALSE;

	if (pNMTreeView->item.pszText && *pNMTreeView->item.pszText)
	{
		IShellFolder* pParentFolder;
		if (SUCCEEDED(SHBindToParent(pItem->pidlFQ, IID_PPV_ARGS(&pParentFolder), NULL)))
		{
			LPITEMIDLIST pidlRel;
			if (SUCCEEDED(pParentFolder->SetNameOf(m_hWnd, pItem->pidlRel, pNMTreeView->item.pszText, SHGDN_NORMAL, &pidlRel)))
			{
				// Tree will be updated through OnShellChange

				*pResult = TRUE;
			}

			pParentFolder->Release();
		}
	}
}

LRESULT CShellTree::OnShellChange(WPARAM wParam, LPARAM lParam)
{
	PIDLIST_ABSOLUTE* pidls;
	LONG Event;
	HANDLE hNotifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &pidls, &Event);

	if (!hNotifyLock)
		return NULL;

	WCHAR Path1[MAX_PATH];
	WCHAR Path2[MAX_PATH];
	WCHAR Parent1[MAX_PATH];
	WCHAR Parent2[MAX_PATH];
	Path1[0] = Path2[0] = Parent1[0] = Parent2[0] = L'\0';

	SHGetPathFromIDList(pidls[0], Path1);

	wcscpy_s(Parent1, MAX_PATH, Path1);
	WCHAR* pChar = wcsrchr(Parent1, L'\\');

	if (pChar<=&Parent1[2])
		pChar = &Parent1[3];

	*pChar = '\0';

	if (pidls[1])
	{
		SHGetPathFromIDList(pidls[1], Path2);

		wcscpy_s(Parent2, MAX_PATH, Path2);
		pChar = wcsrchr(Parent2, L'\\');

		if (pChar<=&Parent2[2])
			pChar = &Parent2[3];

		*pChar = '\0';
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
		if ((Path1[0]!='\0') && DeletePath(Path1))
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

		if ((tag.itemNew.hItem=GetSelectedItem())!=NULL)
		{
			tag.itemNew.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
			GetItem(&tag.itemNew);
		}

		GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
	}

	return NULL;
}
