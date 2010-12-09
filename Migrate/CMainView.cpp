
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "Resource.h"
#include "Migrate.h"
#include "LFCore.h"


// CMainView
//

CMainView::CMainView()
{
	m_IsRootSet = FALSE;
}

INT CMainView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, _pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The file view gets the command first
	if (m_wndTree.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Check application commands
	if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainView::ClearRoot()
{
	m_IsRootSet = FALSE;

	CString caption;
	CString hint;
	ENSURE(caption.LoadString(IDS_NOROOT_CAPTION));
	ENSURE(hint.LoadString(IDS_NOROOT_HINT));

	m_wndExplorerHeader.SetText(caption, hint);

	m_wndTree.ClearRoot();
	m_wndTree.EnableWindow(FALSE);
}

void CMainView::SetRoot(LPITEMIDLIST pidl, BOOL Update, BOOL ExpandAll)
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

	m_wndExplorerHeader.SetText(caption, hint);

	m_wndTree.SetRoot(pidl, Update, ExpandAll);
	m_wndTree.EnableWindow(TRUE);
}

void CMainView::PopulateMigrationList(CMigrationList* ml, LFItemDescriptor* it)
{
	if (m_IsRootSet)
		m_wndTree.PopulateMigrationList(ml, it);
}

void CMainView::UncheckMigrated(CReportList* rl)
{
	if (m_IsRootSet)
		m_wndTree.UncheckMigrated(rl);
}

BOOL CMainView::FoldersChecked()
{
	return m_IsRootSet ? m_wndTree.FoldersChecked() : FALSE;
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
	m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndTree.SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight, rect.Width(), rect.Height()-ExplorerHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_VIEW_SELECTROOT_TASKBAR, OnSelectRoot)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_SELECTROOT_TASKBAR, OnUpdateCommands)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, IDB_TASKS, 1))
		return -1;

	m_wndTaskbar.AddButton(IDM_VIEW_SELECTROOT_TASKBAR, 0);
	m_wndTaskbar.AddButton(IDM_VIEW_EXPAND, 1);
	m_wndTaskbar.AddButton(IDM_VIEW_OPEN, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_VIEW_DELETE, 3);
	m_wndTaskbar.AddButton(IDM_VIEW_RENAME, 4);
	m_wndTaskbar.AddButton(IDM_VIEW_PROPERTIES, 5);
	m_wndTaskbar.AddButton(ID_APP_NEWSTOREMANAGER, 6, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 9, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 10, TRUE, TRUE);

	// Explorer header
	if (!m_wndExplorerHeader.Create(this, 2))
		return -1;

	// Tree
	if (!m_wndTree.Create(this, 3))
		return -1;

	ClearRoot();
	return 0;
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMainView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
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

void CMainView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x==-1) && (pos.y==-1))
	{
		pos.x = pos.y = 0;
		ClientToScreen(&pos);
	}

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_BACKGROUND));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
}

void CMainView::OnSelectRoot()
{
	GetOwner()->SendMessage(WM_COMMAND, IDM_VIEW_SELECTROOT);
}

void CMainView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_IsRootSet);
}
