
// FileDropWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include <io.h>


CFileDropWnd::CFileDropWnd()
	: CGlasWindow()
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

	CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, m_hIcon);

	CRect rect(0, 0, 144, 190);
	return CGlasWindow::Create(WS_MINIMIZEBOX, className, _T("FileDrop"), rect);
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

	return CGlasWindow::PreTranslateMessage(pMsg);
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
	UINT Flags = SWP_NOSIZE | SWP_FRAMECHANGED;

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

		PosX = x;
		PosY = y;
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


BEGIN_MESSAGE_MAP(CFileDropWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_RBUTTONDOWN()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	ON_COMMAND(SC_ALWAYSONTOP, OnAlwaysOnTop)
	ON_COMMAND(ID_APP_CHOOSEDEFAULTSTORE, OnChooseDefaultStore)
	ON_COMMAND(ID_APP_STOREPROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_APP_ABOUT, OnAbout)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnNewStoreManager)
	ON_COMMAND(ID_APP_EXIT, OnQuit)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.WakeupMsg, OnWakeup)
END_MESSAGE_MAP()

int CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Aero
	MARGINS Margins = { -1, -1, -1, -1 };
	UseGlasBackground(Margins);

	// Hintergrundbilder laden
	m_Dropzone.Create(128, 128, ILC_COLOR32, 2, 1);

	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons)
	{
		HICON ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(IDI_STORE_Empty), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
		m_Dropzone.Add(ic);
		DestroyIcon(ic);

		ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(IDI_STORE_Default), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
		m_Dropzone.Add(ic);
		DestroyIcon(ic);

		FreeLibrary(hModIcons);
	}

	// Badge laden
	hModIcons = LoadLibrary(_T("LFCOMMDLG.DLL"));
	if (hModIcons)
	{
		m_hWarning = (HICON)LoadImage(hModIcons, IDI_EXCLAMATION, IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);
		FreeLibrary(hModIcons);
	}

	// Tooltip
	Tooltip.Create(this);

	// Einstellungen laden
	PosX = theApp.GetInt(_T("X"), 10000);
	PosY = theApp.GetInt(_T("Y"), 10000);
	SetWindowRect(PosX, PosY, theApp.GetInt(_T("AlwaysOnTop"), TRUE));

	// SC_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((SC_ALWAYSONTOP & 0xFFF0)==SC_ALWAYSONTOP);
	ASSERT(SC_ALWAYSONTOP<0xF000);

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
	theApp.WriteInt(_T("AlwaysOnTop"), AlwaysOnTop);
	theApp.WriteInt(_T("X"), PosX);
	theApp.WriteInt(_T("Y"), PosY);

	CGlasWindow::OnClose();
}

BOOL CFileDropWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rclient;
	GetClientRect(rclient);

	CRect rlayout;
	GetLayoutRect(rlayout);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rclient.Width();
	dib.bmiHeader.biHeight = -rclient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP bmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(bmp);

	// Hintergrund
	CGlasWindow::OnEraseBkgnd(&dc);

	// Dropzone
	POINT pt = { rlayout.left+(rlayout.Width()-128)/2, rlayout.top };
	SIZE sz = { 128, 128 };
	m_Dropzone.DrawEx(&dc, StoreValid, pt, sz, CLR_NONE, CLR_NONE, StoreValid ? ILD_TRANSPARENT : m_IsAeroWindow ? ILD_BLEND25 : ILD_BLEND50);

	// Text
	CRect rtext(rlayout);
	rtext.top += 120;

	const UINT textflags = DT_VCENTER | DT_CENTER | DT_SINGLELINE;

	if ((m_IsAeroWindow) || (hTheme))
	{
		LOGFONT lf;
		theApp.zGetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lf);
		if (lf.lfHeight<-15)
			lf.lfHeight = -15;

		CFont titleFont;
		titleFont.CreateFontIndirect(&lf);
		CFont* oldFont = dc.SelectObject(&titleFont);

		if (m_IsAeroWindow)
		{
			DTTOPTS opts = { sizeof(DTTOPTS) };
			opts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE | DTT_TEXTCOLOR;
			opts.iGlowSize = 15;

			if (theApp.zDrawThemeTextEx)
				theApp.zDrawThemeTextEx(hTheme, dc, 0, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE, Label, -1, textflags, rtext, &opts);
		}
		else
		{
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

		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(Label, -1, rtext, textflags);

		dc.SelectObject(oldFont);
	}

	// Badge
	if (!StoreValid)
		DrawIconEx(dc, rlayout.right-28, rlayout.top, m_hWarning, 24, 24, 0, NULL, DI_NORMAL);

	pDC->BitBlt(0, 0, rclient.Width(), rclient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(bmp);

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
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);

		MouseInWnd = TRUE;
	}

	CGlasWindow::OnMouseMove(nFlags, point);
}

void CFileDropWnd::OnMouseLeave()
{
	Tooltip.Deactivate();
	MouseInWnd = FALSE;

	CGlasWindow::OnMouseLeave();
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
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = m_hWnd;
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

void CFileDropWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0)
	{
	case SC_ALWAYSONTOP:
		OnAlwaysOnTop();
		break;
	default:
		CGlasWindow::OnSysCommand(nID, lParam);
	}
}

void CFileDropWnd::OnMove(int x, int y)
{
	CGlasWindow::OnMove(x, y);

	CRect rect;
	GetWindowRect(rect);

	if ((rect.left>=-100) && (rect.top>=-100))
	{
		PosX = rect.left;
		PosY = rect.top;
	}
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

LRESULT CFileDropWnd::OnWakeup(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (IsIconic())
		ShowWindow(SW_RESTORE);

	SetForegroundWindow();
	return 24878;
}

void CFileDropWnd::OnQuit()
{
	PostQuitMessage(0);
}
