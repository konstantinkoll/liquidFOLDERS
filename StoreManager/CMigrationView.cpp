
// CMigrationView.cpp: Implementierung der Klasse CMigrationView
//

#include "stdafx.h"
#include "CMigrationView.h"
#include "Resource.h"
#include "StoreManager.h"


// CMigrationView
//

CMigrationView::CMigrationView()
{
	m_IsRootSet = FALSE;
}

INT CMigrationView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, _pParentWnd, nID);
}

BOOL CMigrationView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The file view gets the command first
	if (m_wndTree.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Check application commands
	if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMigrationView::ClearRoot()
{
	m_IsRootSet = FALSE;

	CString caption;
	CString hint;
	ENSURE(caption.LoadString(IDS_NOROOT_CAPTION));
	ENSURE(hint.LoadString(IDS_NOROOT_HINT));

	m_wndHeaderArea.SetText(caption, hint);

	m_wndTree.ClearRoot();
	m_wndTree.EnableWindow(FALSE);
}

void CMigrationView::SetRoot(LPITEMIDLIST pidl, BOOL Update, BOOL ExpandAll)
{
	m_IsRootSet = TRUE;

	CString caption;
	CString hint;
	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((WCHAR*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME)))
	{
		caption = sfi.szDisplayName;
		hint = sfi.szTypeName;
	}

	m_wndHeaderArea.SetText(caption, hint);

	m_wndTree.SetRoot(pidl, Update, ExpandAll);
	m_wndTree.EnableWindow(TRUE);
}

void CMigrationView::PopulateMigrationList(CMigrationList* ml, LFItemDescriptor* it)
{
	if (m_IsRootSet)
		m_wndTree.PopulateMigrationList(ml, it);
}

void CMigrationView::UncheckMigrated(CReportList* rl)
{
	if (m_IsRootSet)
		m_wndTree.UncheckMigrated(rl);
}

BOOL CMigrationView::FoldersChecked()
{
	return m_IsRootSet ? m_wndTree.FoldersChecked() : FALSE;
}

void CMigrationView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
	m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndTree.SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight, rect.Width(), rect.Height()-ExplorerHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMigrationView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_MIGRATION_SELECTROOT, OnSelectRoot)
	ON_UPDATE_COMMAND_UI(IDM_MIGRATION_SELECTROOT, OnUpdateCommands)
END_MESSAGE_MAP()

INT CMigrationView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, IDB_TASKS_32, IDB_TASKS_16, 1))
		return -1;

	m_wndTaskbar.AddButton(IDM_MIGRATION_SELECTROOT, 17);
	m_wndTaskbar.AddButton(IDM_TREE_EXPAND, 16);
	m_wndTaskbar.AddButton(IDM_TREE_OPEN, 17, TRUE);
	m_wndTaskbar.AddButton(IDM_TREE_DELETE, 26);
	m_wndTaskbar.AddButton(IDM_TREE_RENAME, 27);
	m_wndTaskbar.AddButton(IDM_TREE_PROPERTIES, 21);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 31, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 32, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 33, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 34, TRUE, TRUE);

	// Explorer header
	if (!m_wndHeaderArea.Create(this, 2, TRUE))
		return -1;

	// Tree
	if (!m_wndTree.Create(this, 3))
		return -1;

	ClearRoot();
	return 0;
}

BOOL CMigrationView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMigrationView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMigrationView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_wndTree.IsWindowEnabled())
	{
		m_wndTree.SetFocus();
	}
	else
	{
		m_wndTaskbar.SetFocus();
	}
}

void CMigrationView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_TREE));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
}

void CMigrationView::OnSelectRoot()
{
	GetOwner()->SendMessage(WM_COMMAND, IDM_TREE_SELECTROOT);
}

void CMigrationView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_IsRootSet);
}
