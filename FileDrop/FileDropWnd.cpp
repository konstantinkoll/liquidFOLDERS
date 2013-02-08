
// FileDropWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "MenuIcons.h"
#include "Resource.h"
#include "LFCore.h"
#include <io.h>


// FileDropWnd
//

CFileDropWnd::CFileDropWnd()
	: CGlassWindow()
{
	m_AlwaysOnTop = m_StoreValid = m_StoreMounted = m_Hover = FALSE;
}

BOOL CFileDropWnd::Create()
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, theApp.LoadIcon(IDR_APPLICATION));

	CRect rect(0, 0, 144, 190);
	return CGlassWindow::Create(WS_MINIMIZEBOX, className, _T("FileDrop"), rect);
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

		m_PosX = x;
		m_PosY = y;
	}
	else
	{
		Flags |= SWP_NOMOVE;
	}

	m_AlwaysOnTop = TopMost;
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
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	ON_COMMAND(SC_ALWAYSONTOP, OnAlwaysOnTop)
	ON_COMMAND(ID_APP_CHOOSEDEFAULTSTORE, OnChooseDefaultStore)
	ON_COMMAND(ID_APP_OPENSTORE, OnStoreOpen)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_APP_EXIT, OnQuit)
	ON_UPDATE_COMMAND_UI(ID_APP_OPENSTORE, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateCommands)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.m_WakeupMsg, OnWakeup)
END_MESSAGE_MAP()

INT CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Aero
	MARGINS Margins = { -1, -1, -1, -1 };
	UseGlasBackground(Margins);

	// Badge laden
	hWarning = (HICON)LoadImage(GetModuleHandle(_T("LFCOMMDLG.DLL")), IDI_EXCLAMATION, IMAGE_ICON, 24, 24, LR_DEFAULTCOLOR);

	// Tooltip
	m_TooltipCtrl.Create(this);

	// Einstellungen laden
	m_PosX = theApp.GetInt(_T("X"), 10000);
	m_PosY = theApp.GetInt(_T("Y"), 10000);
	SetWindowRect(m_PosX, m_PosY, theApp.GetInt(_T("AlwaysOnTop"), TRUE));

	// SC_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((SC_ALWAYSONTOP & 0xFFF0)==SC_ALWAYSONTOP);
	ASSERT(SC_ALWAYSONTOP<0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		// Always on top
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_ALWAYSONTOP));
		pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (m_AlwaysOnTop ? MF_CHECKED : 0), SC_ALWAYSONTOP, tmpStr);
		pSysMenu->InsertMenu(SC_CLOSE, MF_SEPARATOR | MF_BYCOMMAND);

		// Überflüssige Einträge löschen
		pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu(SC_SIZE, MF_BYCOMMAND);
	}

	// Store
	SendMessage(theApp.p_MessageIDs->DefaultStoreChanged);

	// Initialize Drop
	m_DropTarget.SetOwner(this);
	m_DropTarget.SetStore("", FALSE);
	RegisterDragDrop(GetSafeHwnd(), &m_DropTarget);

	return 0;
}

void CFileDropWnd::OnClose()
{
	theApp.WriteInt(_T("AlwaysOnTop"), m_AlwaysOnTop);
	theApp.WriteInt(_T("X"), m_PosX);
	theApp.WriteInt(_T("Y"), m_PosY);

	CGlassWindow::OnClose();
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

	// Dropzone
	POINT pt = { rlayout.left+(rlayout.Width()-128)/2, rlayout.top };
	SIZE sz = { 128, 128 };
	theApp.m_CoreImageListJumbo.DrawEx(&dc, (m_StoreValid ? LFGetStoreIcon(&m_Store) : IDI_STORE_Unknown)-1, pt, sz, CLR_NONE, CLR_NONE, (m_StoreValid && m_StoreMounted) ? ILD_TRANSPARENT : m_IsAeroWindow ? ILD_BLEND25 : ILD_BLEND50);

	// Text
	CRect rtext(rlayout);
	rtext.top += 120;

	const UINT textflags = DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX;

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
				theApp.zDrawThemeTextEx(hTheme, dc, 0, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE, m_Label, -1, textflags, rtext, &opts);
		}
		else
		{
			theApp.zDrawThemeText(hTheme, dc, WP_CAPTION, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE,
				m_Label, -1, textflags, 0, rtext);

			dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
			dc.DrawText(m_Label, rtext, textflags);
		}

		dc.SelectObject(oldFont);
	}
	else
	{
		HGDIOBJ oldFont = dc.SelectStockObject(DEFAULT_GUI_FONT);

		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_Label, rtext, textflags);

		dc.SelectObject(oldFont);
	}

	// Badge
	if (!m_StoreValid)
		DrawIconEx(dc, rlayout.right-28, rlayout.top, hWarning, 24, 24, 0, NULL, DI_NORMAL);

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

	SetMenuItemBitmap(*pPopup, 8, HBMMENU_POPUP_CLOSE);

	pPopup->SetDefaultItem(ID_APP_CHOOSEDEFAULTSTORE);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);
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

void CFileDropWnd::OnMove(INT x, INT y)
{
	CGlassWindow::OnMove(x, y);

	CRect rect;
	GetWindowRect(rect);

	if ((rect.left>=-100) && (rect.top>=-100))
	{
		m_PosX = rect.left;
		m_PosY = rect.top;
	}
}


LRESULT CFileDropWnd::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_StoreValid = (LFGetStoreSettings("", &m_Store)==LFOk);
	if (m_StoreValid)
	{
		m_StoreMounted = LFIsStoreMounted(&m_Store);
		m_Label = m_Store.StoreName;
	}
	else
	{
		ENSURE(m_Label.LoadString(IDS_NODEFAULTSTORE));
	}

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


// Commands

void CFileDropWnd::OnChooseDefaultStore()
{
	LFChooseStoreDlg dlg(this, LFCSD_ChooseDefault);
	if (dlg.DoModal()==IDOK)
		if (dlg.m_StoreID[0]!='\0')
			LFErrorBox(LFMakeDefaultStore(dlg.m_StoreID, NULL), GetSafeHwnd());
}

void CFileDropWnd::OnStoreOpen()
{
	ShellExecute(GetSafeHwnd(), _T("open"), theApp.m_Path+_T("StoreManager.exe"), CString(m_Store.StoreID), NULL, SW_SHOW);
}

void CFileDropWnd::OnStoreImportFolder()
{
	LFImportFolder(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreProperties()
{
	LFStorePropertiesDlg dlg(m_Store.StoreID, this);
	dlg.DoModal();
}

void CFileDropWnd::OnAlwaysOnTop()
{
	SetWindowRect(-1, -1, !m_AlwaysOnTop);
}

void CFileDropWnd::OnQuit()
{
	PostQuitMessage(0);
}

void CFileDropWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(pCmdUI->m_nID==ID_APP_OPENSTORE ? _waccess(theApp.m_Path+_T("StoreManager.exe"), 0)==0 : m_StoreValid);
}
