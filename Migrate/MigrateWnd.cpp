
// FileDropWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include <io.h>


CMigrateWnd::CMigrateWnd()
	: CGlasWindow()
{
	m_hIcon = NULL;
}

CMigrateWnd::~CMigrateWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CMigrateWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));

	return CGlasWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

void CMigrateWnd::AdjustLayout()
{
	CRect rect;
	GetLayoutRect(rect);

	const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
	m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMigrateWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_RBUTTONDOWN()
	ON_WM_NCHITTEST()
	ON_WM_SYSCOMMAND()
	ON_WM_ACTIVATE()
	ON_WM_MOVE()
	ON_COMMAND(ID_APP_ABOUT, OnAbout)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnNewStoreManager)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

int CMigrateWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Aero
	MARGINS Margins = { 0, 0, 100, 0 };
	UseGlasBackground(Margins);

	// Bottom area
	m_wndBottomArea.Create(this, MAKEINTRESOURCE(IDD_BOTTOMAREA), CBRS_BOTTOM, 1);

	AdjustLayout();
	GetDlgItem(IDOK)->SetFocus();
	return 0;
}

LRESULT CMigrateWnd::OnNcHitTest(CPoint point)
{
	SHORT LButtonDown = GetAsyncKeyState(VK_LBUTTON);
	LRESULT uHitTest = CGlasWindow::OnNcHitTest(point);
	return ((uHitTest==HTCLIENT) && (LButtonDown & 0x8000)) ? HTCAPTION : uHitTest;
}

void CMigrateWnd::OnActivate(UINT /*nState*/, CWnd* /*pWndOther*/, BOOL bMinimized)
{
	if (!bMinimized)
		Invalidate();
}

void CMigrateWnd::OnAbout()
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

void CMigrateWnd::OnNewStoreManager()
{
	theApp.OnAppNewStoreManager();
}

LRESULT CMigrateWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return NULL;
}
