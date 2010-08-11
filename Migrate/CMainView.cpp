
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

void CMainView::SetRoot(CString _Root)
{
	Root = _Root;
	IsRootSet = TRUE;
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

	Invalidate();
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	if (((CGlasWindow*)GetParent())->GetDesign()==GWD_DEFAULT)
		rect.DeflateRect(1, 1);

	const UINT ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
	m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT HeaderHeight = m_wndHeader.GetPreferredHeight();
	m_wndHeader.SetWindowPos(NULL, rect.left, rect.top+ExplorerHeight, rect.Width(), HeaderHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndTree.SetWindowPos(NULL, rect.left, rect.top+ExplorerHeight+HeaderHeight, rect.Width(), rect.Height()-ExplorerHeight-HeaderHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Task bar


	// Explorer header
	m_wndExplorerHeader.Create(this, 2);

	// Column header
	m_wndHeader.Create(this, 3);

	// Tree
	m_wndTree.Create(this, 4);

	ClearRoot();
	return 0;
}

BOOL CMainView::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, 0xA0A0A0);

	return TRUE;
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndTree.SetFocus();
}
