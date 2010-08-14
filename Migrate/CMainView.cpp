
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
	IsRootSet = FALSE;
}

void CMainView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T("MigrateMainView"), dwStyle, rect, _pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainView::SetRoot(CString _Root)
{
	Root = _Root;
	IsRootSet = TRUE;

	m_wndExplorerHeader.SetLineStyle(FALSE, FALSE);

	Invalidate();
}

void CMainView::ClearRoot()
{
	Root.Empty();
	IsRootSet = FALSE;

	CString caption;
	CString hint;
	ENSURE(caption.LoadString(IDS_NOROOT_CAPTION));
	ENSURE(hint.LoadString(IDS_NOROOT_HINT));
	m_wndExplorerHeader.SetText(caption, hint, FALSE);
	m_wndExplorerHeader.SetLineStyle(TRUE, FALSE);

	Invalidate();
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	if (((CGlasWindow*)GetParent())->GetDesign()==GWD_DEFAULT)
		rect.DeflateRect(1, 1);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
	m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT HeaderHeight = m_wndHeader.GetPreferredHeight();
	m_wndHeader.SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight, rect.Width(), HeaderHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndTree.SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight+HeaderHeight, rect.Width(), rect.Height()-ExplorerHeight-HeaderHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SELECTROOT, ID_VIEW_DELETE, OnUpdateTaskbar)
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	m_wndTaskbar.Create(this, IDB_TASKS, 1);

	m_wndTaskbar.AddButton(ID_VIEW_SELECTROOT, _T("Select root folder"), 0);
	m_wndTaskbar.AddButton(ID_VIEW_INCLUDETREE, _T("Include subfolders"), 1);
	m_wndTaskbar.AddButton(ID_VIEW_EXCLUDETREE, _T("Exclude subfolders"), 2);
	m_wndTaskbar.AddButton(ID_VIEW_RENAME, _T("Rename folder"), 3);
	m_wndTaskbar.AddButton(ID_VIEW_DELETE, _T("Delete folder"), 4);
	m_wndTaskbar.AddButton(ID_APP_NEWSTOREMANAGER, _T("Open StoreManager"), 5);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, _T("Purchase license"), 6, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, _T("Enter license key"), 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, _T("Customer support"), 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, _T("About Migration Wizard"), 9, TRUE, TRUE);

	// Explorer header
	m_wndExplorerHeader.Create(this, 2);

	// Column header
	m_wndHeader.Create(this, 3);

	// Tree
	m_wndTree.Create(this, 4);

	ClearRoot();
	return 0;
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMainView::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	pDC.FillSolidRect(rect, 0xA0A0A0);
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

LRESULT CMainView::OnThemeChanged()
{
	AdjustLayout();
	return TRUE;
}

void CMainView::OnUpdateTaskbar(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_SELECTROOT:
		pCmdUI->Enable(!IsRootSet);
		break;
	default:
		pCmdUI->Enable(IsRootSet);
	}
}
