
// FileDropWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include <io.h>
#include <uxtheme.h>


CFileDropWnd::CFileDropWnd()
	: CGlassWindow()
{
	m_hIcon = NULL;
	MouseInWnd = Grabbed = StoreValid = FALSE;
}

CFileDropWnd::~CFileDropWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CFileDropWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, m_hIcon);

	const DWORD dwStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD dwExStyle = WS_EX_APPWINDOW;
	CRect rect(0, 0, 144, 180);
	return CGlassWindow::CreateEx(dwExStyle, className, _T("FileDrop"), dwStyle, rect, NULL, 0);
}

BOOL CFileDropWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
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
		Tooltip.Hide();
		break;
	}

	return CGlassWindow::PreTranslateMessage(pMsg);
}

void CFileDropWnd::UpdateStore()
{
	char* key = LFGetDefaultStore();
	if (*key!='\0')
	{
		StoreValid = (LFGetStoreSettings(key, &m_Store)==LFOk);
	}
	else
	{
		StoreValid = FALSE;
	}
	free(key);

	if (StoreValid)
	{
		Label = m_Store.StoreName;
	}
	else
	{
		Label.LoadString(IDS_NODEFAULTSTORE);
	}
}

void CFileDropWnd::SetWindowRect(int x, int y, BOOL TopMost)
{
	UINT Flags = SWP_NOSIZE;

	if ((x!=-1) || (y!=-1))
	{
		CRect r;
		GetWindowRect(&r);

		CRect d;
		GetDesktopWindow()->GetWindowRect(&d);

		if (x+r.Width()>d.Width())
			x = d.Width()-r.Width();
		if (y+r.Height()>d.Height())
			y = d.Height()-r.Height();
	}
	else
	{
		Flags |= SWP_NOMOVE;
	}

	AlwaysOnTop = TopMost;
	SetWindowPos(TopMost ? &wndTopMost : &wndNoTopMost, x, y, 0, 0, Flags);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
		pSysMenu->CheckMenuItem(SC_ALWAYSONTOP, MF_BYCOMMAND | (TopMost ? MF_CHECKED : MF_UNCHECKED));
}

BEGIN_MESSAGE_MAP(CFileDropWnd, CGlassWindow)
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
	ON_COMMAND(SC_ALWAYSONTOP, OnAlwaysOnTop)
	ON_COMMAND(ID_APP_CHOOSEDEFAULTSTORE, OnChooseDefaultStore)
	ON_COMMAND(ID_APP_STOREPROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_APP_ABOUT, OnAbout)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnNewStoreManager)
	ON_COMMAND(ID_APP_EXIT, OnAbout)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

int CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Hintergrundbilder laden
	m_Dropzone.Create(128, 128, ILC_COLOR32, 2, 1);

	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons)
	{
		HICON ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(IDI_STORE_Empty), IMAGE_ICON, 128, 128, LR_LOADTRANSPARENT);
		m_Dropzone.Add(ic);
		DestroyIcon(ic);

		ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(IDI_STORE_Default), IMAGE_ICON, 128, 128, LR_LOADTRANSPARENT);
		m_Dropzone.Add(ic);
		DestroyIcon(ic);

		FreeLibrary(hModIcons);
	}

	// Badge laden
	m_Warning.Load(IDB_WARNING);
	m_Warning.SetSingleImage();

	// Tooltip
	Tooltip.Create(this);

	// Einstellungen laden
	SetWindowRect(theApp.GetInt(_T("X"), 10000), theApp.GetInt(_T("Y"), 10000), theApp.GetInt(_T("AlwaysONTop"), TRUE));

	// IDM_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ALWAYSONTOP & 0xFFF0)==IDM_ALWAYSONTOP);
	ASSERT(IDM_ALWAYSONTOP<0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		// Always on top
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_ALWAYSONTOP));
		pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (AlwaysOnTop ? MF_CHECKED:0), SC_ALWAYSONTOP, tmpStr);
		pSysMenu->InsertMenu(SC_CLOSE, MF_SEPARATOR | MF_BYCOMMAND);

		// Überflüssige Einträge löschen
		pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu(SC_SIZE, MF_BYCOMMAND);
	}

	// Store
	UpdateStore();

	// Initialize Drop
	m_DropTarget.Register(this, "");

	return 0;
}

void CFileDropWnd::OnClose()
{
	CRect r;
	GetWindowRect(&r);
	theApp.WriteInt(_T("AlwaysOnTop"), AlwaysOnTop);
	theApp.WriteInt(_T("X"), r.left);
	theApp.WriteInt(_T("Y"), r.top);

	CGlassWindow::OnClose();
}

BOOL CFileDropWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// Hintergrund
	COLORREF cr = m_IsAeroWindow ? 0x000000 : GetSysColor(hTheme ? (GetActiveWindow())==this ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION : COLOR_3DFACE);
	dc.FillSolidRect(rect, cr);

	// Dropzone
	POINT pt = { (rect.Width()-128)>>1, 0 };
	SIZE sz = { 128, 128 };
	m_Dropzone.DrawEx(&dc, StoreValid, pt, sz, CLR_NONE, cr, StoreValid ? ILD_TRANSPARENT : m_IsAeroWindow ? ILD_BLEND25 : ILD_BLEND50);

	// Text
	const UINT textflags = DT_VCENTER | DT_CENTER | DT_SINGLELINE;
	CRect rtext;
	rtext.SetRectEmpty();

	if ((m_IsAeroWindow) || (hTheme))
	{
		LOGFONT lf;
		theApp.zGetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lf);
		if (lf.lfHeight<-15)
			lf.lfHeight = -15;

		CFont titleFont;
		titleFont.CreateFontIndirect(&lf);

		CFont* oldFont = dc.SelectObject(&titleFont);
		dc.DrawText(Label, -1, rtext, textflags | DT_CALCRECT);

		if (m_IsAeroWindow)
		{
			wchar_t Buffer[256];
			wcscpy_s(Buffer, 256, Label);
			
			FontFamily fontFamily(lf.lfFaceName);
			StringFormat strformat;
			GraphicsPath TextPath;
			TextPath.AddString(Buffer, (int)wcslen(Buffer), &fontFamily, FontStyleRegular, (REAL)-lf.lfHeight, Gdiplus::Point(((rect.Width()-rtext.Width())>>1)-5, 120+((rect.bottom-120-rtext.Height())>>1)), &strformat);

			Graphics g(dc.m_hDC);
			g.SetCompositingMode(CompositingModeSourceOver);
			g.SetSmoothingMode(SmoothingModeAntiAlias);
			g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

			Pen pen(Color(0x30, 0xFF, 0xFF, 0xFF), 5.5);
			pen.SetLineJoin(LineJoinRound);
			g.DrawPath(&pen, &TextPath);
			pen.SetWidth(3.5);
			g.DrawPath(&pen, &TextPath);
			pen.SetWidth(1.5);
			g.DrawPath(&pen, &TextPath);
			SolidBrush brush(Color(0, 0, 0));
			g.FillPath(&brush, &TextPath);
		}
		else
		{
			rtext.top = 120;
			rtext.bottom = rect.bottom;
			rtext.right = rect.right;

			theApp.zDrawThemeText(hTheme, dc, WP_CAPTION, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE,
				Label, -1, textflags, 0, rtext);

			dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
			dc.DrawText(Label, -1, rtext, textflags);
		}

		dc.SelectObject(oldFont);
	}
	else
	{
		HGDIOBJ oldFont = dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT));
		dc.DrawText(Label, -1, rtext, textflags | DT_CALCRECT);

		rtext.top = 120;
		rtext.bottom = rect.bottom;
		rtext.right = rect.right;

		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(Label, -1, rtext, textflags);

		dc.SelectObject(oldFont);
	}

	// Badge
	if (!StoreValid)
	{
		CAfxDrawState ds;
		m_Warning.PrepareDrawImage(ds, CSize(24, 24));
		m_Warning.Draw(&dc, rect.Width()-28, 0, 0);
		m_Warning.EndDrawImage(ds);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
	buffer.DeleteObject();

	return TRUE;
}

void CFileDropWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!MouseInWnd)
	{
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);

		MouseInWnd = TRUE;
	}

	CGlassWindow::OnMouseMove(nFlags, point);
}

void CFileDropWnd::OnMouseLeave()
{
	Tooltip.Deactivate();
	MouseInWnd = FALSE;

	CGlassWindow::OnMouseLeave();
}

void CFileDropWnd::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		CString strHint;
		ENSURE(strHint.LoadString(IDS_TOOLTIP));

		ClientToScreen(&point);
		Tooltip.Track(point, strHint);
	}
	else
	{
		Tooltip.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}

void CFileDropWnd::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDM_POPUP);

	CMenu* popup = menu.GetSubMenu(0);
	if (popup)
	{
		popup->CheckMenuItem(SC_ALWAYSONTOP, AlwaysOnTop ? MF_CHECKED : MF_UNCHECKED);
		popup->EnableMenuItem(ID_APP_STOREPROPERTIES, StoreValid ? MF_ENABLED : MF_GRAYED);
		popup->EnableMenuItem(ID_APP_NEWSTOREMANAGER, (_access(theApp.path+"StoreManager.exe", 0)==0) ? MF_ENABLED : MF_GRAYED);

		popup->SetDefaultItem(ID_APP_CHOOSEDEFAULTSTORE);
		ClientToScreen(&point);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

LRESULT CFileDropWnd::OnNcHitTest(CPoint point)
{
	SHORT LButtonDown;
	LButtonDown = GetAsyncKeyState(VK_LBUTTON);

	LRESULT uHitTest = DefWindowProc(WM_NCHITTEST, 0, (point.y<<16) | point.x);
	return ((uHitTest==HTCLIENT) && (LButtonDown & 0x8000)) ? HTCAPTION : uHitTest;
}

void CFileDropWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0)
	{
	case SC_ALWAYSONTOP:
		OnAlwaysOnTop();
		break;
	default:
		CGlassWindow::OnSysCommand(nID, lParam);
	}
}

void CFileDropWnd::OnActivate(UINT /*nState*/, CWnd* /*pWndOther*/, BOOL bMinimized)
{
	if (!bMinimized)
		Invalidate();
}

void CFileDropWnd::OnAlwaysOnTop()
{
	SetWindowRect(-1, -1, !AlwaysOnTop);
}

void CFileDropWnd::OnChooseDefaultStore()
{
	LFChooseDefaultStoreDlg dlg(this);
	dlg.DoModal();
}

void CFileDropWnd::OnStoreProperties()
{
	if (StoreValid)
	{
		LFStorePropertiesDlg dlg(m_Store.StoreID, this);
		dlg.DoModal();
	}
}

void CFileDropWnd::OnAbout()
{
	LFAboutDlgParameters p;
	p.appname = "FileDrop";
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

void CFileDropWnd::OnNewStoreManager()
{
	theApp.OnAppNewStoreManager();
}

LRESULT CFileDropWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateStore();
	Invalidate();

	return NULL;
}
