
// CFileDropWnd.cpp: Implementierungsdatei der Klasse CFileDropWnd
//

#include "stdafx.h"
#include "CFileDropWnd.h"
#include "liquidFOLDERS.h"


// CFileDropWnd
//

CFileDropWnd::CFileDropWnd()
	: CGlassWindow()
{
	m_Hover = FALSE;
}

BOOL CFileDropWnd::Create(CHAR* StoreID)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IDR_FILEDROP));

	CString Caption((LPCSTR)IDR_FILEDROP);

	return CGlassWindow::Create(WS_MINIMIZEBOX, className, Caption, _T("FileDrop"), CSize(164, 210));
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
		LFGetApp()->HideTooltip();
		break;
	}

	return CGlassWindow::PreTranslateMessage(pMsg);
}

void CFileDropWnd::SetTopMost(BOOL AlwaysOnTop)
{
	theApp.m_FileDropAlwaysOnTop = m_AlwaysOnTop = AlwaysOnTop;

	SetWindowPos(AlwaysOnTop ? &wndTopMost : &wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

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
	ON_COMMAND(IDM_STORE_SYNCHRONIZE, OnStoreSynchronize)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPENNEWWINDOW, OnUpdateStoreCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_SYNCHRONIZE, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StatisticsChanged, OnUpdateStore)
END_MESSAGE_MAP()

INT CFileDropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Aero
	MARGINS Margins = { -1, -1, -1, -1 };
	UseGlasBackground(Margins);

	// SC_xxx muss sich im Bereich der Systembefehle befinden.
	ASSERT((SC_ALWAYSONTOP & 0xFFF0)==SC_ALWAYSONTOP);
	ASSERT(SC_ALWAYSONTOP<0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		// Always on top
		CString tmpStr((LPCSTR)SC_ALWAYSONTOP);
		pSysMenu->InsertMenu(SC_CLOSE, MF_STRING | MF_BYCOMMAND | (m_AlwaysOnTop ? MF_CHECKED : 0), SC_ALWAYSONTOP, tmpStr);
		pSysMenu->InsertMenu(SC_CLOSE, MF_SEPARATOR | MF_BYCOMMAND);

		// Überflüssige Einträge löschen
		pSysMenu->DeleteMenu(SC_MAXIMIZE, MF_BYCOMMAND);
		pSysMenu->DeleteMenu(SC_SIZE, MF_BYCOMMAND);
	}

	// TopMose
	SetTopMost(theApp.m_FileDropAlwaysOnTop);

	// Store
	OnUpdateStore(NULL, NULL);

	// Initialize Drop
	m_DropTarget.SetOwner(this);
	m_DropTarget.SetStore(m_StoreID, FALSE);
	RegisterDragDrop(GetSafeHwnd(), &m_DropTarget);

	return 0;
}

BOOL CFileDropWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectLayout;
	GetLayoutRect(rectLayout);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	HBITMAP hBitmap = CreateTransparentBitmap(rectClient.Width(), rectClient.Height());
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	// Hintergrund
	CGlassWindow::OnEraseBkgnd(&dc);

	// Icon
	theApp.m_CoreImageListJumbo.DrawEx(&dc, m_StoreIcon-1,
		CPoint(rectLayout.left+(rectLayout.Width()-128)/2-1, rectLayout.top+10), CSize(128, 128),
		CLR_NONE, CLR_NONE, ((m_StoreType & LFTypeNotMounted) ? m_IsAeroWindow ? ILD_BLEND25 : ILD_BLEND50 : ILD_TRANSPARENT) | (m_StoreType & LFTypeBadgeMask));

	// Text
	CRect rectText(rectLayout);
	rectText.top += 130;
	rectText.bottom -= 10;

	const UINT nFormat = DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX;

	if ((m_IsAeroWindow) || (hTheme))
	{
		LOGFONT lf;
		theApp.zGetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lf);
		if (lf.lfHeight<-15)
			lf.lfHeight = -15;

		CFont TitleFont;
		TitleFont.CreateFontIndirect(&lf);

		CFont* pOldFont = dc.SelectObject(&TitleFont);

		if (m_IsAeroWindow)
		{
			DTTOPTS opts = { sizeof(DTTOPTS) };
			opts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE | DTT_TEXTCOLOR;
			opts.iGlowSize = 15;

			if (theApp.zDrawThemeTextEx)
				theApp.zDrawThemeTextEx(hTheme, dc, 0, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE, m_Store.StoreName, -1, nFormat, rectText, &opts);
		}
		else
		{
			theApp.zDrawThemeText(hTheme, dc, WP_CAPTION, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE,
				m_Store.StoreName, -1, nFormat, 0, rectText);

			dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
			dc.DrawText(m_Store.StoreName, -1, rectText, nFormat);
		}

		dc.SelectObject(pOldFont);
	}
	else
	{
		HGDIOBJ hOldFont = dc.SelectStockObject(DEFAULT_GUI_FONT);

		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_Store.StoreName, -1, rectText, nFormat);

		dc.SelectObject(hOldFont);
	}

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

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
	LFGetApp()->HideTooltip();
	m_Hover = FALSE;

	CGlassWindow::OnMouseLeave();
}

void CFileDropWnd::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!LFGetApp()->IsTooltipVisible())
		{
			LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(&m_Store);

			CString Hint;
			GetHintForStore(pItemDescriptor, Hint);

			LFGetApp()->ShowTooltip(this, point, m_Store.StoreName, Hint, LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon(m_StoreIcon-1));

			LFFreeItemDescriptor(pItemDescriptor);
		}
	}
	else
	{
		LFGetApp()->HideTooltip();
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

	CString tmpStr((LPCSTR)IDS_CONTEXTMENU_OPENNEWWINDOW);
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

void CFileDropWnd::OnStoreSynchronize()
{
	LFRunSynchronization(m_StoreID, this);
}

void CFileDropWnd::OnStoreMakeDefault()
{
	LFErrorBox(this, LFSetDefaultStore(m_Store.StoreID));
}

void CFileDropWnd::OnStoreImportFolder()
{
	LFImportFolder(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreShortcut()
{
	LFCreateDesktopShortcutForStoreEx(&m_Store);
}

void CFileDropWnd::OnStoreDelete()
{
	LFDeleteStore(m_Store.StoreID, this);
}

void CFileDropWnd::OnStoreProperties()
{
	LFStorePropertiesDlg dlg(m_Store.StoreID, this);
	dlg.DoModal();
}

void CFileDropWnd::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_STORE_SYNCHRONIZE:
		b = (m_StoreType & LFTypeSynchronizeAllowed);
		break;

	case IDM_STORE_MAKEDEFAULT:
		b = !(m_StoreType & LFTypeDefault);
		break;

	case IDM_STORE_IMPORTFOLDER:
		b = !(m_StoreType & LFTypeNotMounted);
		break;

	case IDM_STORE_SHORTCUT:
		b = (m_StoreType & LFTypeShortcutAllowed);
		break;

	case IDM_STORE_RENAME:
		b = FALSE;
		break;
	}

	pCmdUI->Enable(b);
}


LRESULT CFileDropWnd::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetStoreSettings(m_StoreID, &m_Store)!=LFOk)
		PostMessage(WM_CLOSE);

	m_StoreIcon = LFGetStoreIcon(&m_Store, &m_StoreType);

	SetWindowText(m_Store.StoreName);
	Invalidate();

	return NULL;
}
