
// CFrontstageScroller.cpp: Implementierung der Klasse CFrontstageScroller
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageScroller
//

CFrontstageScroller::CFrontstageScroller(UINT Flags)
	: CFrontstageWnd()
{
	m_Flags = Flags;
	m_HeaderHeight = m_ItemHeight = 0;
	m_szScrollStep.cx = m_szScrollStep.cy = DEFAULTSCROLLSTEP;
	m_pWndHeader = NULL;

	m_szScrollStep.cx = DEFAULTSCROLLSTEP;
	m_szScrollStep.cy = LFGetApp()->m_DefaultFont.GetFontHeight();

	ResetScrollArea();
}

INT CFrontstageScroller::GetHeaderIndent() const
{
	return BACKSTAGEBORDER-1;
}

void CFrontstageScroller::GetHeaderContextMenu(CMenu& /*Menu*/, INT /*HeaderItem*/)
{
}

void CFrontstageScroller::AdjustScrollbars()
{
	// Dimensions
	CRect rect;
	GetWindowRect(rect);

	if (HasBorder())
		rect.DeflateRect(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE));

	BOOL HScroll = FALSE;
	if (m_ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}

	if (m_ScrollHeight>rect.Height()-(INT)m_HeaderHeight)
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);

	if ((m_ScrollWidth>rect.Width()) && !HScroll)
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	// Set vertical bars
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	// Set horizontal bars
	m_HScrollMax = max(0, m_ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	si.nPage = rect.Width();
	si.nMax = m_ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);
}

void CFrontstageScroller::AdjustLayout()
{
	// Get header layout
	WINDOWPOS wp;
	ZeroMemory(&wp, sizeof(wp));

	if (m_pWndHeader)
	{
		CRect rect;
		GetWindowRect(rect);

		HDLAYOUT HdLayout;
		HdLayout.prc = &rect;
		HdLayout.pwpos = &wp;
		m_pWndHeader->Layout(&HdLayout);

		wp.x = GetHeaderIndent();
		wp.y = 0;
		m_HeaderHeight = wp.cy;
	}

	// Scrolling
	AdjustScrollbars();
	Invalidate();

	// Set header position
	if (m_pWndHeader)
		m_pWndHeader->SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);

	// Update hover item
	UpdateHoverItem();
}

void CFrontstageScroller::ScrollWindow(INT dx, INT dy, LPCRECT /*lpRect*/, LPCRECT /*lpClipRect*/)
{
	// Items
	const BOOL Themed = IsCtrlThemed();

	if (Themed && (m_Flags & FRONTSTAGE_COMPLEXBACKGROUND))
	{
		Invalidate();
	}
	else
	{
		CRect rect;
		GetClientRect(rect);

		rect.top = m_HeaderHeight;

		if (Themed && (dy!=0))
		{
			rect.bottom -= BACKSTAGERADIUS;
	
			ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
			RedrawWindow(CRect(rect.left, rect.bottom, rect.right, rect.bottom+BACKSTAGERADIUS), NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else
		{
			ScrollWindowEx(dx, dy, rect, rect, NULL, NULL, SW_INVALIDATE);
		}
	}

	// Header
	if (m_pWndHeader && (dx!=0))
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		WINDOWPOS wp;
		HDLAYOUT HdLayout;
		HdLayout.prc = &rectWindow;
		HdLayout.pwpos = &wp;
		m_pWndHeader->Layout(&HdLayout);

		wp.x = GetHeaderIndent();
		wp.y = 0;

		m_pWndHeader->SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
		m_pWndHeader->UpdateWindow();
	}
}

void CFrontstageScroller::GetNothingMessage(CString& strMessage, COLORREF& /*clrMessage*/, BOOL /*Themed*/) const
{
	ENSURE(strMessage.LoadString(IDS_NOTHINGTODISPLAY));
}

BOOL CFrontstageScroller::DrawNothing() const
{
	return FALSE;
}

BOOL CFrontstageScroller::DrawNothing(CDC& dc, LPCRECT lpRectClient, BOOL Themed) const
{
	if (DrawNothing())
	{
		// Message
		CString strMessage;
		COLORREF clrMessage = Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW);

		GetNothingMessage(strMessage, clrMessage, Themed);

		// Draw message
		CRect rectText(lpRectClient);
		rectText.top += m_HeaderHeight+BACKSTAGEBORDER;

		if (Themed)
		{
			rectText.top++;

			dc.SetTextColor(0xFFFFFF);
			dc.DrawText(strMessage, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

			rectText.top--;
		}

		dc.SetTextColor(clrMessage);
		dc.DrawText(strMessage, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		return TRUE;
	}

	return FALSE;
}

void CFrontstageScroller::DrawStage(CDC& /*dc*/, Graphics& /*g*/, const CRect& /*rect*/, const CRect& /*rectUpdate*/, BOOL /*Themed*/)
{
}

void CFrontstageScroller::ResetScrollArea()
{
	m_ScrollWidth = m_ScrollHeight = m_HScrollMax = m_VScrollMax = m_HScrollPos = m_VScrollPos = 0;
	m_HoverItem = -1;
}

void CFrontstageScroller::SetItemHeight(INT ItemHeight, INT Gutter)
{
	m_szScrollStep.cy = (m_ItemHeight=ItemHeight)+Gutter;
}


BEGIN_MESSAGE_MAP(CFrontstageScroller, CFrontstageWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_ENABLE()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

void CFrontstageScroller::OnDestroy()
{
	if (m_pWndHeader)
	{
		m_pWndHeader->DestroyWindow();
		delete m_pWndHeader;
	}

	CFrontstageWnd::OnDestroy();
}

void CFrontstageScroller::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	Graphics g(dc);

	// Background
	const BOOL Themed = IsCtrlThemed();

	if ((m_Flags & FRONTSTAGE_CARDBACKGROUND)==FRONTSTAGE_CARDBACKGROUND)
	{
		DrawCardBackground(dc, g, CRect(rect.left, rect.top+m_HeaderHeight, rect.right, rect.bottom), Themed);
	}
	else
	{
		dc.FillSolidRect(rect, IsWindowEnabled() ? Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW) : GetSysColor(COLOR_3DFACE));
	}

	// Stage
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	if (!DrawNothing(dc, rect, Themed))
		DrawStage(dc, g, rect, rectUpdate, Themed);

	dc.SelectObject(pOldFont);

	// Header
	if (m_HeaderHeight)
		if (Themed)
		{
			dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, 0xFFFFFF);

			Bitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVUP);

			g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2+m_HScrollPos+BACKSTAGEBORDER+CARDPADDING-1, m_HeaderHeight-(INT)pDivider->GetHeight());
		}
		else
		{
			dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, GetSysColor(COLOR_3DFACE));
		}

	// Edge
	DrawWindowEdge(g, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CFrontstageScroller::OnEnable(BOOL /*bEnable*/)
{
	Invalidate();
}

void CFrontstageScroller::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CFrontstageScroller::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;

	case SB_LINEUP:
		nInc = -m_szScrollStep.cy;
		break;

	case SB_LINEDOWN:
		nInc = m_szScrollStep.cy;
		break;

	case SB_PAGEUP:
		nInc = min(-1, -(rect.Height()-(INT)m_HeaderHeight));
		break;

	case SB_PAGEDOWN:
		nInc = max(1, rect.Height()-(INT)m_HeaderHeight);
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		SetScrollPos(SB_VERT, m_VScrollPos+=nInc);
		ScrollWindow(0, -nInc);
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CFrontstageScroller::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_HScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_HScrollMax-m_HScrollPos;
		break;

	case SB_LINEUP:
		nInc = -m_szScrollStep.cx;
		break;

	case SB_LINEDOWN:
		nInc = m_szScrollStep.cx;
		break;

	case SB_PAGEUP:
		nInc = min(-1, -rect.Width());
		break;

	case SB_PAGEDOWN:
		nInc = max(1, rect.Width());
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &si);

		nInc = si.nTrackPos-m_HScrollPos;
		break;
	}

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		SetScrollPos(SB_HORZ, m_HScrollPos+=nInc);
		ScrollWindow(-nInc, 0);
	}

	CFrontstageWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CFrontstageScroller::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*m_szScrollStep.cy*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CFrontstageScroller::OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*m_szScrollStep.cx/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindow(-nInc, 0);
		SetScrollPos(SB_HORZ, m_HScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CFrontstageScroller::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

void CFrontstageScroller::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

void CFrontstageScroller::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_pWndHeader && (pWnd==m_pWndHeader))
	{
		// Header item clicked
		CPoint pt(point);
		ScreenToClient(&pt);

		HDHITTESTINFO htt;
		htt.pt = pt;

		// Get menu
		CMenu Menu;
		GetHeaderContextMenu(Menu, m_pWndHeader->HitTest(&htt));

		// Track popup menu
		TrackPopupMenu(Menu, point, this, FALSE);
	}
	else
	{
		CFrontstageWnd::OnContextMenu(pWnd, point);
	}
}
void CFrontstageScroller::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CFrontstageScroller::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}
