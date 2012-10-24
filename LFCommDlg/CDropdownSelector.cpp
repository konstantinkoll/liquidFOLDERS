
// CDropdownSelector.cpp: Implementierung der Klasse CDropdownSelector
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CDropdownListCtrl
//

CDropdownListCtrl::CDropdownListCtrl()
	: CExplorerList()
{
}


BEGIN_MESSAGE_MAP(CDropdownListCtrl, CExplorerList)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CDropdownListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CExplorerList::OnMouseMove(nFlags, point);

	INT idx = HitTest(point);
	if (idx!=-1)
	{
		SetItemState(idx, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

		if (GetFocus()!=this)
			SetFocus();
	}
}

void CDropdownListCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
}

void CDropdownListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	CExplorerList::OnLButtonUp(nFlags, point);

	INT idx = HitTest(point);
	if (idx!=-1)
		GetOwner()->SendMessage(WM_SETITEM, (WPARAM)idx);
}

void CDropdownListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar==VK_EXECUTE) || (nChar==VK_RETURN))
	{
		INT idx = GetNextItem(-1, LVIS_SELECTED);
		if (idx!=-1)
			GetOwner()->SendMessage(WM_SETITEM, (WPARAM)idx);
	}
	else
	{
		CExplorerList::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


// CDropdownWindow
//

CDropdownWindow::CDropdownWindow()
	: CWnd()
{
}

BOOL CDropdownWindow::Create(CWnd* pParentWnd, CRect rectDrop, UINT DialogResID)
{
	m_DialogResID = DialogResID;

	UINT nClassStyle = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	if (LFGetApp()->OSVersion>OS_XP)
	{
		BOOL bDropShadow;
		SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);
		if (bDropShadow)
			nClassStyle |= CS_DROPSHADOW;
	}

	CString className = AfxRegisterWndClass(nClassStyle, LoadCursor(NULL, IDC_ARROW));
	BOOL res = CWnd::CreateEx(WS_EX_CONTROLPARENT | WS_EX_TOPMOST, className, _T(""), WS_BORDER | WS_VISIBLE | WS_POPUP, rectDrop.left, rectDrop.top, rectDrop.Width(), rectDrop.Height(), pParentWnd->GetSafeHwnd(), NULL);

	SetOwner(pParentWnd);

	return res;
}

BOOL CDropdownWindow::PreTranslateMessage(MSG* pMsg)
{
	CWnd* pWnd;

	if (pMsg->message==WM_KEYDOWN)
		switch (pMsg->wParam)
		{
		case VK_TAB:
			pWnd = GetNextDlgTabItem(GetFocus(), GetKeyState(VK_SHIFT)<0);
			if (pWnd)
				pWnd->SetFocus();
			return TRUE;
		case VK_ESCAPE:
			GetOwner()->PostMessage(WM_CLOSEDROPDOWN);
			return TRUE;
		}

	return CWnd::PreTranslateMessage(pMsg);
}

void CDropdownWindow::AdjustLayout()
{
	if (!IsWindow(m_wndList))
		return;

	CRect rect;
	GetClientRect(rect);

	if (IsWindow(m_wndBottomArea))
	{
		const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
		m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);
		rect.bottom -= BottomHeight;
	}

	m_wndList.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndList.EnsureVisible(0, FALSE);
}


BEGIN_MESSAGE_MAP(CDropdownWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()

INT CDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;

	CRect rect;
	rect.SetRectEmpty();
	if (m_wndList.Create(dwStyle | WS_TABSTOP | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_SINGLESEL, rect, this, 1)==-1)
		return -1;

	m_wndList.SetExtendedStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS);
	m_wndList.SetFont(&LFGetApp()->m_DefaultFont, FALSE);

	BOOL Themed = IsCtrlThemed();
	m_wndList.SetBkColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));
	m_wndList.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
	m_wndList.SetTextBkColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	if (m_DialogResID)
		if (m_wndBottomArea.Create(this, m_DialogResID, CBRS_BOTTOM, 2)==-1)
			return -1;

	return 0;
}

void CDropdownWindow::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDropdownWindow::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndList.SetFocus();
}

void CDropdownWindow::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	CWnd::OnActivateApp(bActive, dwThreadID);

	if (!bActive)
		GetOwner()->PostMessage(WM_CLOSEDROPDOWN);
}


// CDropdownSelector
//

#define BORDER     4

CDropdownSelector::CDropdownSelector()
	: CWnd()
{
	p_App = (LFApplication*)AfxGetApp();
	p_DropWindow = NULL;
	hTheme = NULL;
	m_Icon = NULL;
	m_IsEmpty = TRUE;
	m_Hover = m_Pressed = m_Dropped = FALSE;
}

BOOL CDropdownSelector::Create(CString EmptyHint, CString Caption, CGlassWindow* pParentWnd, UINT nID)
{
	m_EmptyHint = EmptyHint;
	m_Caption = Caption;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CDropdownSelector::PreTranslateMessage(MSG* pMsg)
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

	return CWnd::PreTranslateMessage(pMsg);
}

void CDropdownSelector::CreateDropdownWindow(CRect rectDrop)
{
	p_DropWindow = new CDropdownWindow();
	((CGlassWindow*)GetParent())->RegisterPopupWindow(p_DropWindow);
	p_DropWindow->Create(this, rectDrop);
}

void CDropdownSelector::NotifyOwner(UINT NotifyCode)
{
	NMHDR tag;
	tag.hwndFrom = m_hWnd;
	tag.idFrom = GetDlgCtrlID();
	tag.code = NotifyCode;

	GetOwner()->SendMessage(WM_NOTIFY, tag.idFrom, LPARAM(&tag));
}

void CDropdownSelector::SetEmpty(BOOL Repaint)
{
	OnCloseDropdown();
	if (m_Icon)
	{
		DestroyIcon(m_Icon);
		m_Icon = NULL;
	}

	BOOL bNotifyOwner = !m_IsEmpty;
	m_IsEmpty = TRUE;

	if (Repaint)
		Invalidate();

	if (bNotifyOwner)
		NotifyOwner(NM_SELCHANGED);
}

void CDropdownSelector::SetItem(HICON hIcon, CString DisplayName, BOOL Repaint, UINT NotifyCode)
{
	OnCloseDropdown();
	if (m_Icon)
		DestroyIcon(m_Icon);

	m_Icon = hIcon;
	m_DisplayName = DisplayName;
	m_IsEmpty = FALSE;

	if (Repaint)
		Invalidate();

	NotifyOwner(NotifyCode);
}

void CDropdownSelector::GetTooltipData(HICON& /*hIcon*/, CSize& /*Size*/, CString& /*Caption*/, CString& /*Hint*/)
{
}

UINT CDropdownSelector::GetPreferredHeight()
{
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	UINT h = max(dc->GetTextExtent(_T("Wy")).cy, GetSystemMetrics(SM_CYSMICON));
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	return h+2*BORDER;
}

BOOL CDropdownSelector::IsEmpty()
{
	return m_IsEmpty;
}


BEGIN_MESSAGE_MAP(CDropdownSelector, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_MESSAGE(WM_OPENDROPDOWN, OnOpenDropdown)
	ON_MESSAGE(WM_CLOSEDROPDOWN, OnCloseDropdown)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

INT CDropdownSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	OnThemeChanged();

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

void CDropdownSelector::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);
	if (m_Icon)
		DestroyIcon(m_Icon);

	OnCloseDropdown();
	CWnd::OnDestroy();
}

BOOL CDropdownSelector::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CDropdownSelector::OnPaint()
{
	CPaintDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rectClient.Width();
	dib.bmiHeader.biHeight = -rectClient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmp);

	Graphics g(dc);

	CGlassWindow* pCtrlSite = (CGlassWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);
	const BYTE Alpha = m_Dropped || (p_App->OSVersion==OS_Eight) ? 0xFF : (m_Hover || (GetFocus()==this)) ? 0xF0 : 0xD0;

	CRect rectContent(rectClient);
	if (hTheme)
	{
		CRect rectBounds(rectContent);
		rectBounds.right--;
		rectBounds.bottom--;

		rectContent.DeflateRect(2, 2);
		SolidBrush brush1(Color(Alpha, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush1, rectContent.left, rectContent.top, rectContent.Width(), rectContent.Height());
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);
		Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
		g.DrawPath(&pen, &path);
		rectBounds.DeflateRect(1, 1);

		if (LFGetApp()->OSVersion==OS_Eight)
		{
			CreateRoundRectangle(rectBounds, 0, path);
			Pen pen(Color(0x40, 0x38, 0x38, 0x38));
			g.DrawPath(&pen, &path);
		}
		else
		{
			CreateRoundRectangle(rectBounds, 1, path);
			LinearGradientBrush brush2(Point(0, rectBounds.top), Point(0, rectBounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
			pen.SetBrush(&brush2);
			g.DrawPath(&pen, &path);
		}
	}
	else
	{
		dc.Draw3dRect(rectContent, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
		rectContent.DeflateRect(1, 1);
		if (m_Dropped || (GetFocus()==this))
		{
			dc.Draw3dRect(rectContent, 0x000000, GetSysColor(COLOR_3DFACE));
			rectContent.DeflateRect(1, 1);
			dc.FillSolidRect(rectContent, GetSysColor(COLOR_WINDOW));
		}
		else
		{
			rectContent.DeflateRect(1, 1);
		}
	}

	CRect rectClip(rectContent);
	rectClip.left = rectClip.right-GetSystemMetrics(SM_CXHSCROLL);
	CRect rectArrow(rectClip);

	if (hTheme)
	{
		// Hack to achieve the same style as Windows Explorer
		if (p_App->OSVersion>OS_XP)
		{
			rectArrow.InflateRect(1, 1);
			if ((m_Hover) && (p_App->OSVersion!=OS_Eight))
			{
				rectClip.left--;
			}
			else
			{
				rectArrow.InflateRect(1, 1);
			}
		}

		p_App->zDrawThemeBackground(hTheme, dc, CP_DROPDOWNBUTTON, m_Pressed ? CBXS_PRESSED : (m_Hover && !m_Dropped) ? CBXS_HOT : CBXS_NORMAL, rectArrow, rectClip);
	}
	else
	{
		if (m_Hover || m_Pressed || m_Dropped)
			dc.DrawFrameControl(rectArrow, DFC_BUTTON, DFCS_TRANSPARENT | 16 | DFCS_HOT | (m_Pressed ? DFCS_PUSHED : 0));

		dc.DrawFrameControl(rectArrow, DFC_MENU, DFCS_TRANSPARENT | 16 | (m_Pressed ? DFCS_PUSHED : (m_Hover && !m_Dropped) ? DFCS_HOT : DFCS_FLAT));
		rectClip.left--;

		if (m_Hover || m_Pressed || m_Dropped)
			dc.FillSolidRect(rectClip.left, rectClip.top, 1, rectClip.Height(), GetSysColor(COLOR_3DFACE));
	}

	CRect rectText(rectContent);
	rectText.right = rectClip.left;
	rectText.DeflateRect(BORDER, 0);

	CFont* pOldFont;

	if (m_IsEmpty)
	{
		pOldFont = dc.SelectObject(&p_App->m_ItalicFont);
		dc.SetTextColor(0x808080);
		dc.DrawText(m_EmptyHint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		pOldFont = dc.SelectObject(&p_App->m_DefaultFont);
		COLORREF c1 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000;
		COLORREF c2 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_3DSHADOW) : 0x808080;

		if (!m_Caption.IsEmpty())
		{
			dc.SetTextColor((m_Hover || m_Dropped) ? c1 : c2);
			dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
			rectText.left += dc.GetTextExtent(m_Caption).cx+BORDER;
		}

		if (m_Icon)
		{
			INT cx = GetSystemMetrics(SM_CXSMICON);
			INT cy = GetSystemMetrics(SM_CYSMICON);

			if (rectText.left+cx<rectText.right)
			{
				DrawIconEx(dc, rectText.left, rectText.top+(rectText.Height()-cy)/2, m_Icon, cx, cy, 0, NULL, DI_NORMAL);
				rectText.left += cx+BORDER;
			}
		}

		if (!m_DisplayName.IsEmpty())
		{
			dc.SetTextColor(c1);
			dc.DrawText(m_DisplayName, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
		}
	}

	dc.SelectObject(pOldFont);

	if ((GetFocus()==this) && (!m_Dropped) && (!hTheme))
	{
		rectText.InflateRect(3, -1);
		dc.SetTextColor(0x000000);
		dc.DrawFocusRect(rectText);
	}

	// Set alpha
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);
	BYTE* pBits = ((BYTE*)bmp.bmBits)+4*(rectContent.top*rectClient.Width()+rectContent.left);
	for (INT row=rectContent.top; row<rectContent.bottom; row++)
	{
		for (INT col=rectContent.left; col<rectContent.right; col++)
		{
			*(pBits+3) = Alpha;
			pBits += 4;
		}
		pBits += 4*(rectClient.Width()-rectContent.Width());
	}

	pDC.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBmp);
}

LRESULT CDropdownSelector::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_COMBOBOX);
	}

	return TRUE;
}

void CDropdownSelector::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CDropdownSelector::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	Invalidate();
}

void CDropdownSelector::OnMouseHover(UINT nFlags, CPoint point)
{
	if (((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0) && (!m_Dropped) && (!m_IsEmpty))
	{
		if (!m_TooltipCtrl.IsWindowVisible())
		{
			HICON hIcon = NULL;
			CSize size(0, 0);
			CString caption;
			CString hint;
			GetTooltipData(hIcon, size, caption, hint);

			ClientToScreen(&point);
			m_TooltipCtrl.Track(point, hIcon, size, caption, hint);
		}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}
}

void CDropdownSelector::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();

	if (m_Dropped)
	{
		OnCloseDropdown();
	}
	else
	{
		m_Pressed = TRUE;
		OnOpenDropdown();
	}
}

void CDropdownSelector::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_Pressed)
	{
		m_Pressed = FALSE;
		Invalidate();

		ReleaseCapture();
	}
}

void CDropdownSelector::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect rect;
	GetWindowRect(rect);
	point += rect.TopLeft();
	GetParent()->ScreenToClient(&point);

	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, (LPARAM)((point.y<<16) | point.x));
}

void CDropdownSelector::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	switch(nChar)
	{
	case VK_SPACE:
	case VK_RETURN:
	case VK_DOWN:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0) && (!m_Dropped))
		{
			OnOpenDropdown();
			ReleaseCapture();
		}
		break;
	case VK_ESCAPE:
	case VK_UP:
	case VK_TAB:
		if (m_Dropped)
			OnCloseDropdown();
		break;
	}
}

LRESULT CDropdownSelector::OnOpenDropdown(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_Dropped)
		return FALSE;

	m_Dropped = TRUE;
	SetCapture();

	Invalidate();
	UpdateWindow();

	CRect rectClient;
	GetClientRect(rectClient);
	ClientToScreen(rectClient);

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(rectClient.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	CRect rectDrop(rectClient);
	rectDrop.DeflateRect(1, 1);
	rectDrop.top = rectDrop.bottom;
	rectDrop.bottom = rectDrop.top+rectScreen.Height()*2/5;
	if (rectDrop.Width()<500)
		rectDrop.right = rectDrop.left+500;
	if (rectDrop.bottom>rectScreen.bottom)
		rectDrop.MoveToY(rectClient.top-rectDrop.Height()+1);

	CreateDropdownWindow(rectDrop);

	return TRUE;
}

LRESULT CDropdownSelector::OnCloseDropdown(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_TooltipCtrl.Deactivate();

	if (p_DropWindow)
	{
		((CGlassWindow*)GetParent())->RegisterPopupWindow(NULL);

		p_DropWindow->DestroyWindow();
		delete p_DropWindow;
		p_DropWindow = NULL;

		m_Dropped = FALSE;
		SetFocus();
		GetParent()->Invalidate();
		GetParent()->UpdateWindow();		// Essential, as parent window's redraw flag may be false

		return TRUE;
	}

	return FALSE;
}

void CDropdownSelector::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CDropdownSelector::OnKillFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
	UpdateWindow();							// Essential, as parent window's redraw flag may be false
}
