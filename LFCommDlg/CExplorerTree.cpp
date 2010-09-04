
// CExplorerTree.cpp: Implementierung der Klasse CExplorerTree
//

#include "stdafx.h"
#include "CExplorerTree.h"


extern AFX_EXTENSION_MODULE LFCommDlgDLL;


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

CExplorerTree::CExplorerTree()
	: CTreeCtrl()
{
	p_App = (LFApplication*)AfxGetApp();
	m_pContextMenu2 = NULL;
}

BOOL CExplorerTree::Create(CWnd* pParentWnd, UINT nID, BOOL OnlyFilesystem)
{
	m_OnlyFilesystem = OnlyFilesystem;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS | TVS_NOTOOLTIPS;
	CRect rect;
	rect.SetRectEmpty();
	return CTreeCtrl::Create(dwStyle, rect, pParentWnd, nID);
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

void CExplorerTree::PopulateTree()
{
	LPITEMIDLIST pidl;
	if (FAILED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl)))
		return;

	TV_ITEM tvItem;
	tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
	pItem->pidlRel = pidl;
	pItem->pidlFQ = p_App->GetShellManager()->CopyItem(pidl);
	pItem->pParentFolder = NULL;
	tvItem.lParam = (LPARAM)pItem;

	CString strItem = OnGetItemText(pItem);
	tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
	tvItem.iImage = OnGetItemIcon(pItem, FALSE);
	tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);
	tvItem.cChildren = TRUE;

	TV_INSERTSTRUCT tvInsert;
	tvInsert.item = tvItem;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.hParent = TVI_ROOT;
	HTREEITEM hParentItem = InsertItem(&tvInsert);

	Expand(hParentItem, TVE_EXPAND);
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

		TVITEM tvItem;
		ZeroMemory(&tvItem, sizeof(tvItem));
		tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
		pParentFolder->AddRef();

		LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));
		pItem->pidlRel = pidlTemp;
		pItem->pidlFQ = p_App->GetShellManager()->ConcatenateItem(pidlParent, pidlTemp);
		pItem->pParentFolder = pParentFolder;
		tvItem.lParam = (LPARAM)pItem;

		CString strItem = OnGetItemText(pItem);
		tvItem.pszText = strItem.GetBuffer(strItem.GetLength());
		tvItem.iImage = OnGetItemIcon(pItem, FALSE);
		tvItem.iSelectedImage = OnGetItemIcon(pItem, TRUE);

		tvItem.cChildren = (dwAttribs & SFGAO_HASSUBFOLDER);
		if (dwAttribs & SFGAO_SHARE)
		{
			tvItem.mask |= TVIF_STATE;
			tvItem.stateMask |= TVIS_OVERLAYMASK;
			tvItem.state |= INDEXTOOVERLAYMASK(1);
		}

		TVINSERTSTRUCT tvInsert;
		tvInsert.item = tvItem;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.hParent = hParentItem;
		InsertItem(&tvInsert);
	}

	pEnum->Release();
}


BEGIN_MESSAGE_MAP(CExplorerTree, CTreeCtrl)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
END_MESSAGE_MAP()

int CExplorerTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
		p_App->zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);

	if (p_App->OSVersion==OS_XP)
		ModifyStyle(0, TVS_HASLINES);

	LOGFONT lf;
	p_App->m_DefaultFont.GetLogFont(&lf);
	SetItemHeight((SHORT)(max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON))+(p_App->OSVersion<OS_Vista ? 2 : 6)));
	SetBkColor(0xFFFFFF);
	SetTextColor(0x000000);
	SetImageList(&p_App->m_SystemImageListSmall, 0);

	PopulateTree();

	return 0;
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
		ENSURE(SUCCEEDED(SHGetDesktopFolder(&psfFolder)));
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
