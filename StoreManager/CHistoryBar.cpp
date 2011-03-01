
#include "stdafx.h"
#include "CHistoryBar.h"
#include "StoreManager.h"


// Breadcrumbs
//

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, FVPersistentData& data)
{
	BreadcrumbItem* add = new BreadcrumbItem;
	add->next = *bi;
	add->filter = f;
	add->data = data;
	*bi = add;
}

void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, FVPersistentData* data)
{
	*f = NULL;
	ZeroMemory(data, sizeof(FVPersistentData));

	if (*bi)
	{
		*f = (*bi)->filter;
		*data = (*bi)->data;
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		delete victim;
	}
}

void DeleteBreadcrumbs(BreadcrumbItem** bi)
{
	while (*bi)
	{
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		LFFreeFilter(victim->filter);
		delete victim;
	}
}


// CHistoryBar
//

#define BORDER          4

CHistoryBar::CHistoryBar()
	: CWnd()
{
	hTheme = NULL;
	m_Hover = FALSE;
}

BOOL CHistoryBar::Create(CGlasWindow* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;// | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CHistoryBar::PreTranslateMessage(MSG* pMsg)
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

UINT CHistoryBar::GetPreferredHeight()
{
	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	UINT h = max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON));

	return h+2*BORDER;
}


BEGIN_MESSAGE_MAP(CHistoryBar, CWnd)
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
	//ON_WM_SETFOCUS()
	//ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

INT CHistoryBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	OnThemeChanged();

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

void CHistoryBar::OnDestroy()
{
	if (hTheme)
		theApp.zCloseThemeData(hTheme);

	CWnd::OnDestroy();
}

BOOL CHistoryBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHistoryBar::OnPaint()
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

	CGlasWindow* pCtrlSite = (CGlasWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);
	const BYTE Alpha = /*m_Dropped ? 0xFF : */(m_Hover || (GetFocus()==this)) ? 0xF0 : 0xD0;

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

		CreateRoundRectangle(rectBounds, 1, path);
		LinearGradientBrush brush2(Point(0, rectBounds.top), Point(0, rectBounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
		pen.SetBrush(&brush2);
		g.DrawPath(&pen, &path);
	}
	else
	{
		dc.Draw3dRect(rectContent, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
		rectContent.DeflateRect(1, 1);
		if (/*m_Dropped || */(GetFocus()==this))
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

	/*CRect rectClip(rectContent);
	rectClip.left = rectClip.right-GetSystemMetrics(SM_CXHSCROLL);
	CRect rectArrow(rectClip);

	if (hTheme)
	{
		// Hack to achieve the same style as Windows Explorer
		if (p_App->OSVersion>OS_XP)
		{
			rectArrow.InflateRect(1, 1);
			if (m_Hover)
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
	}*/

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

LRESULT CHistoryBar::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hTheme)
			theApp.zCloseThemeData(hTheme);

		hTheme = theApp.zOpenThemeData(m_hWnd, VSCLASS_COMBOBOX);
	}

	return TRUE;
}

void CHistoryBar::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
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

void CHistoryBar::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	Invalidate();
}

void CHistoryBar::OnMouseHover(UINT nFlags, CPoint point)
{
	if (((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0) /*&& (!m_Dropped)*/)
	{
		if (!m_TooltipCtrl.IsWindowVisible())
		{
			/*HICON hIcon = NULL;
			CSize size(0, 0);
			CString caption;
			CString hint;
			GetTooltipData(hIcon, size, caption, hint);

			ClientToScreen(&point);
			m_TooltipCtrl.Track(point, hIcon, size, caption, hint);*/
		}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}
}

void CHistoryBar::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
/*	SetFocus();

	if (m_Dropped)
	{
		OnCloseDropdown();
	}
	else
	{
		m_Pressed = TRUE;
		OnOpenDropdown();
	}*/
}

void CHistoryBar::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	/*if (m_Pressed)
	{
		m_Pressed = FALSE;
		Invalidate();

		ReleaseCapture();
	}*/
}

void CHistoryBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect rect;
	GetWindowRect(rect);
	point += rect.TopLeft();
	GetParent()->ScreenToClient(&point);

	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, (LPARAM)((point.y<<16) | point.x));
}

/*void CHistoryBar::OnSetFocus(CWnd* pOldWnd)
{
	Invalidate();
}

void CHistoryBar::OnKillFocus(CWnd* pOldWnd)
{
	if (!OnCloseDropdown())
		Invalidate();
}*/
