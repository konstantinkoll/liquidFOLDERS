
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
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_THEMECHANGED()
	ON_BN_CLICKED(ID_APP_ABOUT, OnAbout)
	ON_BN_CLICKED(ID_APP_NEWSTOREMANAGER, OnNewStoreManager)
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Task bar
	m_wndTaskbar.Create(this, IDB_TASKS, 1);

	m_wndTaskbar.AddButton(100, _T("Select root folder"), 0);
	m_wndTaskbar.AddButton(101, _T("Include subfolders"), 1);
	m_wndTaskbar.AddButton(102, _T("Exclude subfolders"), 2);
	m_wndTaskbar.AddButton(103, _T("Rename folder"), 3);
	m_wndTaskbar.AddButton(104, _T("Delete folder"), 4);
	m_wndTaskbar.AddButton(ID_APP_NEWSTOREMANAGER, _T("Open StoreManager"), 5);

	m_wndTaskbar.AddButton(ID_APP_HELP, _T("Help"), 10, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_PURCHASE, _T("Customer support"), 9, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, _T("About Migration Wizard"), 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_PURCHASE, _T("Enter license key"), 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_PURCHASE, _T("Purchase license"), 6, TRUE, TRUE);

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

LRESULT CMainView::OnThemeChanged()
{
	AdjustLayout();
	return TRUE;
}

void CMainView::OnAbout()
{
	LFAboutDlgParameters p;
	ENSURE(p.appname.LoadString(IDR_APPLICATION));
	p.build = __TIMESTAMP__;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());
	p.TextureSize = -1;
	p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
	p.HideEmptyDrives = -1;
	p.HideEmptyDomains = -1;

	LFAboutDlg dlg(&p, this);
	dlg.DoModal();

	delete p.icon;
}

void CMainView::OnNewStoreManager()
{
	theApp.OnAppNewStoreManager();
}
