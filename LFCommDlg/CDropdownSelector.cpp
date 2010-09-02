
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
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CDropdownListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if ((!hTheme) && (GetHotItem()==(int)lplvcd->nmcd.dwItemSpec))
		{
			lplvcd->nmcd.uItemState |= CDIS_SELECTED;
			lplvcd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
			lplvcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);

			SetBkColor(lplvcd->clrTextBk);

			CRect rect;
			GetItemRect(lplvcd->nmcd.dwItemSpec, rect, LVIR_BOUNDS);
			::FillRect(lplvcd->nmcd.hdc, rect, CreateSolidBrush(lplvcd->clrTextBk));
			*pResult = CDRF_NOTIFYPOSTPAINT;
			break;
		}

		*pResult = CDRF_DODEFAULT;
		break;
	case CDDS_ITEMPOSTPAINT:
		if ((!hTheme) && (GetHotItem()==(int)lplvcd->nmcd.dwItemSpec))
		{
			SetBkColor(GetSysColor(COLOR_WINDOW));

			CRect rect;
			GetItemRect(lplvcd->nmcd.dwItemSpec, rect, LVIR_BOUNDS);
			DrawFocusRect(lplvcd->nmcd.hdc, rect);

			*pResult = CDRF_SKIPDEFAULT;
			break;
		}
	default:
		*pResult = CDRF_DODEFAULT;
	}
}

void CDropdownListCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
}

void CDropdownListCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
}

void CDropdownListCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	int idx = GetHotItem();
	if (idx!=-1)
	{
		CRect rect;
		GetItemRect(idx, rect, LVIR_BOUNDS);

		if (rect.PtInRect(point))
			SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED);
	}
}


// CDropdownWindow
//

CDropdownWindow::CDropdownWindow()
	: CWnd()
{
}

BOOL CDropdownWindow::Create(CWnd* pOwnerWnd, UINT _DialogResID)
{
	m_DialogResID = _DialogResID;

	CString className = AfxRegisterWndClass(CS_DROPSHADOW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	BOOL res = CWnd::CreateEx(WS_EX_CONTROLPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, className, _T(""), WS_BORDER | WS_CHILD, 0, 0, 0, 0, GetDesktopWindow()->GetSafeHwnd(), NULL, NULL);
	SetOwner(pOwnerWnd);
	return res;
}

void CDropdownWindow::AdjustLayout()
{
	if (!IsWindow(m_wndList.GetSafeHwnd()))
		return;

	CRect rect;
	GetClientRect(rect);

	if (IsWindow(m_wndBottomArea.GetSafeHwnd()))
	{
		const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
		m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);
		rect.bottom -= BottomHeight;
	}

	m_wndList.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndList.EnsureVisible(0, FALSE);
}

void CDropdownWindow::SetDesign(UINT _Design)
{
	m_wndList.SetBkColor(_Design==GWD_DEFAULT ? GetSysColor(COLOR_WINDOW) : 0xFFFFFF);
	m_wndList.SetTextColor(_Design==GWD_DEFAULT ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000);
	m_wndList.SetTextBkColor(_Design==GWD_DEFAULT ? GetSysColor(COLOR_WINDOW) : 0xFFFFFF);

	if (IsWindow(m_wndBottomArea.GetSafeHwnd()))
		m_wndBottomArea.SetDesign(_Design);
}

void CDropdownWindow::AddCategory(int ID, CString name, CString hint)
{
	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.iGroupId = ID;
	lvg.pszHeader = name.GetBuffer();
	if ((!hint.IsEmpty()) && (((LFApplication*)AfxGetApp())->OSVersion>=OS_Vista))
	{
		lvg.pszSubtitle = hint.GetBuffer();
		lvg.mask |= LVGF_SUBTITLE;
	}

	m_wndList.InsertGroup(ID, &lvg);
}


BEGIN_MESSAGE_MAP(CDropdownWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CDropdownWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS;

	CRect rect;
	rect.SetRectEmpty();
	m_wndList.Create(dwStyle, rect, this, 1);
	m_wndList.SetExtendedStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_ONECLICKACTIVATE | LVS_EX_JUSTIFYCOLUMNS);
	m_wndList.SetHotCursor(LoadCursor(NULL, IDC_ARROW));
	m_wndList.SetFont(&((LFApplication*)AfxGetApp())->m_DefaultFont, FALSE);

	if (m_DialogResID)
		m_wndBottomArea.Create(this, m_DialogResID, dwStyle, 2);

	return 0;
}

void CDropdownWindow::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDropdownWindow::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndList.SetFocus();
}


// CDropdownSelector
//

#define BORDER          4

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

BOOL CDropdownSelector::Create(CString EmptyHint, CGlasWindow* pParentWnd, UINT nID)
{
	m_EmptyHint = EmptyHint;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T("DropdownSelector"), dwStyle, rect, pParentWnd, nID);
}

BOOL CDropdownSelector::PreTranslateMessage(MSG* pMsg)
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
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CDropdownSelector::CreateDropdownWindow()
{
	p_DropWindow = new CDropdownWindow();
	p_DropWindow->Create(this, 0);
}

void CDropdownSelector::SetEmpty(BOOL Repaint)
{
	OnCloseDropdown();
	if (m_Icon)
	{
		DestroyIcon(m_Icon);
		m_Icon = NULL;
	}

	m_IsEmpty = TRUE;

	if (Repaint)
		Invalidate();
}

void CDropdownSelector::SetItem(CString Caption, HICON hIcon, CString DisplayName, BOOL Repaint)
{
	OnCloseDropdown();
	if (m_Icon)
		DestroyIcon(m_Icon);

	m_Caption = Caption;
	m_Icon = hIcon;
	m_DisplayName = DisplayName;
	m_IsEmpty = FALSE;

	if (Repaint)
		Invalidate();
}

void CDropdownSelector::GetTooltipData(HICON& /*hIcon*/, CSize& /*size*/, CString& /*caption*/, CString& /*hint*/)
{
}

UINT CDropdownSelector::GetPreferredHeight()
{
	LOGFONT lf;
	p_App->m_DefaultFont.GetLogFont(&lf);
	UINT h = max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON));

	return h+2*BORDER;
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
	ON_MESSAGE(WM_CLOSEDROPDOWN, OnCloseDropdown)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

int CDropdownSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

	CRect rclient;
	GetClientRect(rclient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rclient.Width();
	dib.bmiHeader.biHeight = -rclient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP bmp = CreateDIBSection(dc.m_hDC, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(bmp);

	Graphics g(dc);

	CGlasWindow* pCtrlSite = (CGlasWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rclient);
	const BYTE Alpha = m_Dropped ? 0xFF : (m_Hover || (GetFocus()==this)) ? 0xF0 : 0xD0;

	CRect rcontent(rclient);
	switch (pCtrlSite->GetDesign())
	{
	case GWD_AERO:
	case GWD_THEMED:
		{
			CRect rbounds(rcontent);
			rbounds.right--;
			rbounds.bottom--;

			rcontent.DeflateRect(2, 2);
			SolidBrush brush1(Color(Alpha, 0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush1, rcontent.left, rcontent.top, rcontent.Width(), rcontent.Height());
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			GraphicsPath path;
			CreateRoundRectangle(rbounds, 2, path);
			Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
			rbounds.DeflateRect(1, 1);

			CreateRoundRectangle(rbounds, 1, path);
			LinearGradientBrush brush2(Point(0, rbounds.top), Point(0, rbounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
			pen.SetBrush(&brush2);
			g.DrawPath(&pen, &path);

			break;
		}
	case GWD_DEFAULT:
		{
			dc.Draw3dRect(rcontent, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
			rcontent.DeflateRect(1, 1);
			if (m_Dropped || (GetFocus()==this))
			{
				dc.Draw3dRect(rcontent, 0x000000, GetSysColor(COLOR_3DFACE));
				rcontent.DeflateRect(1, 1);
				dc.FillSolidRect(rcontent, GetSysColor(COLOR_WINDOW));
			}
			else
			{
				rcontent.DeflateRect(1, 1);
			}

			break;
		}
	}

	CRect rclip(rcontent);
	rclip.left = rclip.right-GetSystemMetrics(SM_CXHSCROLL);
	CRect rarrow(rclip);

	if (hTheme)
	{
		// Hack to achive the same style as Windows Explorer
		if (p_App->OSVersion>OS_XP)
		{
			rarrow.InflateRect(1, 1);
			if (m_Hover)
			{
				rclip.left--;
			}
			else
			{
				rarrow.InflateRect(1, 1);
			}
		}

		p_App->zDrawThemeBackground(hTheme, dc, CP_DROPDOWNBUTTON, m_Pressed ? CBXS_PRESSED : (m_Hover && !m_Dropped) ? CBXS_HOT : CBXS_NORMAL, rarrow, rclip);
	}
	else
	{
		if (m_Hover || m_Pressed || m_Dropped)
			dc.DrawFrameControl(rarrow, DFC_BUTTON, DFCS_TRANSPARENT | 16 | DFCS_HOT | (m_Pressed ? DFCS_PUSHED : 0));

		dc.DrawFrameControl(rarrow, DFC_MENU, DFCS_TRANSPARENT | 16 | (m_Pressed ? DFCS_PUSHED : (m_Hover && !m_Dropped) ? DFCS_HOT : DFCS_FLAT));
		rclip.left--;

		if (m_Hover || m_Pressed || m_Dropped)
			dc.FillSolidRect(rclip.left, rclip.top, 1, rclip.Height(), GetSysColor(COLOR_3DFACE));
	}

	CRect rtext(rcontent);
	rtext.right = rclip.left;
	rtext.DeflateRect(BORDER, 0);

	CFont* pOldFont;

	if (m_IsEmpty)
	{
		pOldFont = dc.SelectObject(&p_App->m_ItalicFont);
		dc.SetTextColor(0x808080);
		dc.DrawText(m_EmptyHint, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		pOldFont = dc.SelectObject(&p_App->m_DefaultFont);
		COLORREF c1 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000;
		COLORREF c2 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_3DSHADOW) : 0x808080;

		if (!m_Caption.IsEmpty())
		{
			dc.SetTextColor((m_Hover || m_Dropped) ? c1 : c2);
			dc.DrawText(m_Caption, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
			rtext.left += dc.GetTextExtent(m_Caption, m_Caption.GetLength()).cx+BORDER;
		}

		if (m_Icon)
		{
			int cx = GetSystemMetrics(SM_CXSMICON);
			int cy = GetSystemMetrics(SM_CYSMICON);

			if (rtext.left+cx<rtext.right)
			{
				DrawIconEx(dc, rtext.left, rtext.top+(rtext.Height()-cy)/2, m_Icon, cx, cy, 0, NULL, DI_NORMAL);
				rtext.left += cx+BORDER;
			}
		}

		if (!m_DisplayName.IsEmpty())
		{
			dc.SetTextColor(c1);
			dc.DrawText(m_DisplayName, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
		}
	}

	dc.SelectObject(pOldFont);

	if ((GetFocus()==this) && (!m_Dropped) && (pCtrlSite->GetDesign()==GWD_DEFAULT))
	{
		rtext.InflateRect(3, -1);
		dc.SetTextColor(0x000000);
		dc.DrawFocusRect(rtext);
	}

	// Set alpha
	BITMAP bm;
	GetObject(bmp, sizeof(BITMAP), &bm);
	BYTE* pBits = ((BYTE*)bm.bmBits)+4*(rcontent.top*rclient.Width()+rcontent.left);
	for (int row=rcontent.top; row<rcontent.bottom; row++)
	{
		for (int col=rcontent.left; col<rcontent.right; col++)
		{
			*(pBits+3) = Alpha;
			pBits += 4;
		}
		pBits += 4*(rclient.Width()-rcontent.Width());
	}

	pDC.BitBlt(0, 0, rclient.Width(), rclient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(bmp);
}

LRESULT CDropdownSelector::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_COMBOBOX);
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
		tme.dwHoverTime = HOVER_DEFAULT;
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
		HICON hIcon = NULL;
		CSize size(0, 0);
		CString caption;
		CString hint;
		GetTooltipData(hIcon, size, caption, hint);

		ClientToScreen(&point);
		m_TooltipCtrl.Track(point, hIcon, size, caption, hint);
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
		m_Pressed = m_Dropped = TRUE;
		SetCapture();

		CreateDropdownWindow();

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

		p_DropWindow->SetDesign(((CGlasWindow*)GetParent())->GetDesign());
		p_DropWindow->SetWindowPos(&wndTopMost, rectDrop.left, rectDrop.top, rectDrop.Width(), rectDrop.Height(), SWP_SHOWWINDOW | SWP_NOACTIVATE);
		((CGlasWindow*)GetParent())->RegisterPopupWindow(p_DropWindow);

		Invalidate();
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

LRESULT CDropdownSelector::OnCloseDropdown(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_TooltipCtrl.Deactivate();

	if (p_DropWindow)
	{
		((CGlasWindow*)GetParent())->RegisterPopupWindow(NULL);
		m_Dropped = FALSE;

		p_DropWindow->DestroyWindow();
		delete p_DropWindow;
		p_DropWindow = NULL;

		Invalidate();
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
}
