store

// FileDropWnd.cpp: Implementierungsdatei der Klasse FileDropWnd
//

#include "stdafx.h"
#include "FileDropWnd.h"
#include "MigrationWnd.h"
#include "StoreManager.h"


// FileDropWnd
//

CFileDropWnd::CFileDropWnd()
	: CGlassWindow()
{
	m_StoreValid = m_StoreMounted = m_Hover = FALSE;
}

BOOL CFileDropWnd::Create(CHAR* StoreID)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, theApp.LoadIcon(IDR_FILEDROP));

	CString caption;
	ENSURE(caption.LoadString(IDR_FILEDROP));

	return CGlassWindow::Create(WS_MINIMIZEBOX, className, caption, _T("FileDrop"), CSize(164, 210));
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

	return CGlassWindow::PreTranslateMessage(pMsg);
}

void CFileDropWnd::SetTopMost(BOOL AlwaysOnTop)
{
	theApp.m_FileDropAlwaysOnTop = m_AlwaysOnTop = AlwaysOnTop;

	SetWindowPos(AlwaysOnTop ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
		pSysMenu->CheckMenuItem(SC_ALWAYSONTOP, MF_BYCOMMAND | (AlwaysOnTop ? MF_CHECKED : MF_UNCHECKED));
}


BEGIN_MESSAGE_MAP(CFileDropWnd, CGlassWindow)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_OPENFILEDROP, OnOpenFileDrop)
	ON_COMMAND(IDM_ITEM_OPENNEWWINDOW, OnStoreOpen)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_MIGRATIONWIZARD, OnStoreMigrationWizard)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPENNEWWINDOW, OnUpdateStoreCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnUpdateStore)
END_MESSAGE_MAP()

INT CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Aero
	MARGINS Margins = { -1, -1, -1, -1 };
	UseGlasBackground(Margins);

	// Tooltip
	m_TooltipCtrl.Create(this);

	// SC_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((SC_ALWAYSONTOP & 0xFFF0)==SC_ALWAYSONTOP);
	ASSERT(SC_ALWAYSONTOP<0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		// Always on top
		CString tmpStr;
		ENSURE(tmpStr.LoadString(SC_ALWAYSONTOP));
		pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (m_AlwaysOnTop ? MF_CHECKED : 0), SC_ALWAYSONTOP, tmpStr);
		pSysMenu->InsertMenu(SC_CLOSE, MF_SEPARATOR | MF_BYCOMMAND);

		// Überflüssige Einträge löschen
		pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu(SC_SIZE, MF_BYCOMMAND);
	}

	// TopMose
	SetTopMost(theApp.m_FileDropAlwaysOnTop);

	// Store
	SendMessage(theApp.p_MessageIDs->StoresChanged);

	// Initialize Drop
	m_DropTarget.SetOwner(this);
	m_DropTarget.SetStore(m_StoreID, FALSE);
	RegisterDragDrop(GetSafeHwnd(), &m_DropTarget);

	return 0;
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

	HBITMAP hBmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmp);

	// Hintergrund
	CGlassWindow::OnEraseBkgnd(&dc);

	// Icon
	POINT pt = { rlayout.left+(rlayout.Width()-128)/2-1, rlayout.top+10 };
	SIZE sz = { 128, 128 };
	theApp.m_CoreImageListJumbo.DrawEx(&dc, (m_StoreValid ? LFGetStoreIcon(&m_Store) : IDI_STR_Unknown)-1, pt, sz, CLR_NONE, CLR_NONE, (m_StoreValid && m_StoreMounted) ? ILD_TRANSPARENT : m_IsAeroWindow ? ILD_BLEND25 : ILD_BLEND50);

	// Text
	CRect rtext(rlayout);
	rtext.top += 130;
	rtext.bottom -= 10;

	const UINT nFormat = DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX;

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
				theApp.zDrawThemeTextEx(hTheme, dc, 0, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE, m_Label, -1, nFormat, rtext, &opts);
		}
		else
		{
			theApp.zDrawThemeText(hTheme, dc, WP_CAPTION, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE,
				m_Label, -1, nFormat, 0, rtext);

			dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
			dc.DrawText(m_Label, rtext, nFormat);
		}

		dc.SelectObject(oldFont);
	}
	else
	{
		HGDIOBJ oldFont = dc.SelectStockObject(DEFAULT_GUI_FONT);

		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_Label, rtext, nFormat);

		dc.SelectObject(oldFont);
	}

	pDC->BitBlt(0, 0, rclient.Width(), rclient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBmp);

	return TRUE;
}

void CFileDropWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_Hover)
	{
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);

		m_Hover = TRUE;
	}

	CGlassWindow::OnMouseMove(nFlags, point);
}

void CFileDropWnd::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;

	CGlassWindow::OnMouseLeave();
}

void CFileDropWnd::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!m_TooltipCtrl.IsWindowVisible())
		{
			CString caption;
			CString hint;
			ENSURE(caption.LoadString(IDR_FILEDROP));
			ENSURE(hint.LoadString(IDS_DROPTIP));

			ClientToScreen(&point);
			m_TooltipCtrl.Track(point,
				(HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_FILEDROP), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR),
				CSize(48, 48), caption, hint);
		}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}
}

void CFileDropWnd::OnNcLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	OnStoreOpen();
}

void CFileDropWnd::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFileDropWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_STORE));

	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENNEWWINDOW));
	pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENNEWWINDOW, tmpStr);

	pPopup->SetDefaultItem(IDM_ITEM_OPENNEWWINDOW);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);
}

void CFileDropWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	CGlassWindow::OnSysCommand(nID, lParam);

	if ((nID & 0xFFF0)==SC_ALWAYSONTOP)
		SetTopMost(!m_AlwaysOnTop);
}

LRESULT CFileDropWnd::OnOpenFileDrop(WPARAM wParam, LPARAM /*lParam*/)
{
	ASSERT(wParam);

	if (m_StoreValid)
		if (strcmp((CHAR*)wParam, m_StoreID)==0)
		{
			if (IsIconic())
				ShowWindow(SW_RESTORE);

			SetForegroundWindow();
			return 24878;
		}

	return NULL;
}


void CFileDropWnd::OnStoreOpen()
{
	CMainWnd* pFrame = new CMainWnd();
	pFrame->CreateStore(m_Store.StoreID);
	pFrame->ShowWindow(SW_SHOW);
}

void CFileDropWnd::OnStoreMakeDefault()
{
	LFErrorBox(LFMakeDefaultStore(m_Store.StoreID, NULL), GetSafeHwnd());
}

void CFileDropWnd::OnStoreImportFolder()
{
	LFImportFolder(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreMigrationWizard()
{
	CMigrationWnd* pFrame = new CMigrationWnd();
	pFrame->Create(m_Store.StoreID);
	pFrame->ShowWindow(SW_SHOW);
}

void CFileDropWnd::OnStoreShortcut()
{
	if (LFAskCreateShortcut(GetSafeHwnd()))
		LFCreateDesktopShortcutForStore(m_Store.StoreID);
}

void CFileDropWnd::OnStoreDelete()
{
	LFErrorBox(theApp.DeleteStore(&m_Store, this), GetSafeHwnd());
}

void CFileDropWnd::OnStoreProperties()
{
	LFStorePropertiesDlg dlg(m_Store.StoreID, this);
	dlg.DoModal();
}

void CFileDropWnd::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = m_StoreValid;
	CHAR StoreID[LFKeySize];

	switch (pCmdUI->m_nID)
	{
	case IDM_STORE_MAKEDEFAULT:
		LFGetDefaultStore(StoreID);
		b &= (strcmp(m_Store.StoreID, StoreID)!=0);
		break;
	case IDM_STORE_IMPORTFOLDER:
	case IDM_STORE_MIGRATIONWIZARD:
		b &= m_StoreMounted;
		break;
	case IDM_STORE_SHORTCUT:
		b &= ((m_Store.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal);
		break;
	case IDM_STORE_RENAME:
		b = FALSE;
		break;
	}

	pCmdUI->Enable(b);
}


LRESULT CFileDropWnd::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_StoreValid = (LFGetStoreSettings(m_StoreID, &m_Store)==LFOk);
	if (m_StoreValid)
	{
		m_StoreMounted = LFIsStoreMounted(&m_Store);
		m_Label = m_Store.StoreName;

		SetWindowText(m_Label);
		Invalidate();
	}
	else
	{
		PostMessage(WM_CLOSE);
	}

	return NULL;
}
