
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

int CMainView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T("MigrateMainView"), dwStyle, rect, _pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
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
}

void CMainView::SetRoot(LPITEMIDLIST pidl, BOOL Update)
{
	m_IsRootSet = TRUE;

	CString caption;
	CString hint;
	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME)))
	{
		caption = sfi.szDisplayName;
		hint = sfi.szTypeName;
	}

	m_wndExplorerHeader.SetText(caption, hint);

	m_wndTree.SetRoot(pidl, Update);
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
	ON_COMMAND(ID_VIEW_SELECTROOT, OnSelectRoot)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SELECTROOT, ID_VIEW_PROPERTIES, OnUpdateTaskbar)
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, IDB_TASKS, 1))
		return -1;

	m_wndTaskbar.AddButton(ID_VIEW_SELECTROOT, _T("Select root folder"), 0);
	m_wndTaskbar.AddButton(ID_VIEW_INCLUDETREE, _T("Include subfolders"), 1);
	m_wndTaskbar.AddButton(ID_VIEW_EXCLUDETREE, _T("Exclude subfolders"), 2);
	m_wndTaskbar.AddButton(ID_VIEW_RENAME, _T("Rename folder"), 3);
	m_wndTaskbar.AddButton(ID_VIEW_DELETE, _T("Delete folder"), 4);
	m_wndTaskbar.AddButton(ID_VIEW_PROPERTIES, _T("Properties"), 5);
	m_wndTaskbar.AddButton(ID_APP_NEWSTOREMANAGER, _T("Open StoreManager"), 6);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, _T("Purchase license"), 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, _T("Enter license key"), 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, _T("Customer support"), 9, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, _T("About liquidFOLDERS"), 10, TRUE, TRUE);

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

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainView::OnSelectRoot()
{
	GetParent()->SendMessage(WM_COMMAND, ID_VIEW_SELECTROOT);
}

void CMainView::OnUpdateTaskbar(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_SELECTROOT:
		pCmdUI->Enable(!m_IsRootSet);
		break;
	default:
		pCmdUI->Enable(m_IsRootSet);
	}
}
