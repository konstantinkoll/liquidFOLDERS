
// FileDropWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include "MenuIcons.h"
#include <io.h>


// FileDropWnd
//

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

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect(0, 0, 144, 190);
	return CGlasWindow::Create(WS_MINIMIZEBOX, className, _T("FileDrop"), rect);
}

BOOL CFileDropWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
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
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CGlasWindow::PreTranslateMessage(pMsg);
}

void CFileDropWnd::UpdateStore()
{
	CHAR* key = LFGetDefaultStore();
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

void CFileDropWnd::SetWindowRect(INT x, INT y, BOOL TopMost)
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
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	ON_COMMAND(SC_ALWAYSONTOP, OnAlwaysOnTop)
	ON_COMMAND(ID_APP_CHOOSEDEFAULTSTORE, OnChooseDefaultStore)
	ON_COMMAND(ID_APP_IMPORTFOLDER, OnImportFolder)
	ON_COMMAND(ID_APP_STOREPROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_APP_ABOUT, OnAbout)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnNewStoreManager)
	ON_COMMAND(ID_APP_EXIT, OnQuit)
	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_CHOOSEDEFAULTSTORE, ID_APP_STOREPROPERTIES, OnUpdateCommands)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.WakeupMsg, OnWakeup)
END_MESSAGE_MAP()

INT CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Aero
	MARGINS Margins = { -1, -1, -1, -1 };
	UseGlasBackground(Margins);

	// Hintergrundbilder laden
	if (!m_Dropzone.Create(128, 128, ILC_COLOR32, 2, 1))
		return -1;

	HICON ic = LFGetIcon(IDI_STORE_Internal, 128, 128);
	m_Dropzone.Add(ic);
	DestroyIcon(ic);

	ic = LFGetIcon(IDI_STORE_Default, 128, 128);
	m_Dropzone.Add(ic);
	DestroyIcon(ic);

	// Badge laden
	HMODULE hModIcons = LoadLibrary(_T("LFCOMMDLG.DLL"));
	if (hModIcons)
	{
		m_hWarning = (HICON)LoadImage(hModIcons, IDI_EXCLAMATION, IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);
		FreeLibrary(hModIcons);
	}

	// Tooltip
	m_TooltipCtrl.Create(this);

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
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);

		MouseInWnd = TRUE;
	}

	CGlasWindow::OnMouseMove(nFlags, point);
}

void CFileDropWnd::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	MouseInWnd = FALSE;

	CGlasWindow::OnMouseLeave();
}

void CFileDropWnd::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!m_TooltipCtrl.IsWindowVisible())
		{
			CString strHint;
			ENSURE(strHint.LoadString(IDS_TOOLTIP));

			ClientToScreen(&point);
			m_TooltipCtrl.Track(point,
				(HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR),
				CSize(48, 48), _T("FileDrop"), strHint);
		}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}
}

void CFileDropWnd::OnNcLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	OnChooseDefaultStore();
}

void CFileDropWnd::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFileDropWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_POPUP));

	if ((pos.x==-1) || (pos.y==-1))
	{
		pos.x = pos.y = 0;
		ClientToScreen(&pos);
	}

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	HBITMAP bmp = SetMenuItemIcon(*pPopup, 7, ID_APP_NEWSTOREMANAGER);
	SetMenuItemBitmap(*pPopup, 8, HBMMENU_POPUP_CLOSE);

	pPopup->CheckMenuItem(SC_ALWAYSONTOP, AlwaysOnTop ? MF_CHECKED : MF_UNCHECKED);
	//pPopup->EnableMenuItem(ID_APP_IMPORTFOLDER, StoreValid ? MF_ENABLED : MF_GRAYED);
	//pPopup->EnableMenuItem(ID_APP_STOREPROPERTIES, StoreValid ? MF_ENABLED : MF_GRAYED);
	//pPopup->EnableMenuItem(ID_APP_NEWSTOREMANAGER, (_waccess(theApp.m_Path+_T("StoreManager.exe"), 0)==0) ? MF_ENABLED : MF_GRAYED);

	pPopup->SetDefaultItem(ID_APP_CHOOSEDEFAULTSTORE);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);

	DeleteObject(bmp);
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

void CFileDropWnd::OnMove(INT x, INT y)
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
	LFChooseStoreDlg dlg(this, LFCSD_ChooseDefault);
	if (dlg.DoModal()==IDOK)
		if (dlg.StoreID[0]!='\0')
			LFErrorBox(LFMakeDefaultStore(dlg.StoreID, NULL), m_hWnd);
}

void CFileDropWnd::OnImportFolder()
{
	if (StoreValid)
		LFImportFolder(m_Store.StoreID, this);
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
	p.AppName = "FileDrop";
	p.Build = __TIMESTAMP__;
	p.Icon = new CGdiPlusBitmapResource();
	p.Icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());
	p.TextureSize = -1;

	LFAboutDlg dlg(&p, this);
	dlg.DoModal();

	delete p.Icon;
}

void CFileDropWnd::OnNewStoreManager()
{
	theApp.OnAppNewStoreManager();
}

void CFileDropWnd::OnQuit()
{
	PostQuitMessage(0);
}

void CFileDropWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((pCmdUI->m_nID==ID_APP_CHOOSEDEFAULTSTORE) ? TRUE : StoreValid);
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
