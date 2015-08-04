
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskButton
//

#define BORDER     4

CTaskButton::CTaskButton()
	: CButton()
{
	m_Hover = FALSE;
}

BOOL CTaskButton::Create(CWnd* pParentWnd, UINT nID, CString Caption, CString TooltipHeader, CString TooltipHint, CMFCToolBarImages* Icons, INT IconSize, INT IconID)
{
	m_Caption = Caption;
	m_TooltipHeader = TooltipHeader;
	m_TooltipHint = TooltipHint;
	p_Icons = Icons;
	m_IconSize = IconSize;
	m_IconID = IconID;
	m_OverlayID = -1;

	CRect rect;
	rect.SetRectEmpty();
	return CButton::Create(TooltipHeader, WS_VISIBLE | WS_DISABLED | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, rect, pParentWnd, nID);
}

BOOL CTaskButton::PreTranslateMessage(MSG* pMsg)
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

	return CButton::PreTranslateMessage(pMsg);
}

void CTaskButton::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}

void CTaskButton::SetIconID(INT IconID, INT OverlayID)
{
	m_IconID = IconID;
	m_OverlayID = OverlayID;

	Invalidate();
}

INT CTaskButton::GetPreferredWidth()
{
	INT l = 2*(BORDER+2)+1;

	if ((p_Icons) && (m_IconID!=-1))
		l += m_IconSize+(m_Caption.IsEmpty() ? 0 : BORDER);

	if (!m_Caption.IsEmpty())
	{
		CDC* pDC = GetDC();
		HFONT hOldFont = (HFONT)pDC->SelectObject(IsCtrlThemed() ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
		l += pDC->GetTextExtent(m_Caption).cx;
		pDC->SelectObject(hOldFont);
		ReleaseDC(pDC);
	}

	return l;
}


BEGIN_MESSAGE_MAP(CTaskButton, CButton)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

INT CTaskButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

BOOL CTaskButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTaskButton::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// State
	BOOL Focused = (GetState() & 8);
	BOOL Selected = (GetState() & 4);

	// Background
	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	// Button
	BOOL Themed = IsCtrlThemed();
	if (Themed)
	{
		if ((Focused) || (Selected) || (m_Hover))
		{
			Graphics g(dc);
			g.SetSmoothingMode(SmoothingModeNone);

			CRect rectBounds(rect);
			rectBounds.right--;
			rectBounds.bottom--;

			// Inner Border
			rectBounds.DeflateRect(1, 1);

			if (Selected)
			{
				SolidBrush brush(Color(0x20, 0x50, 0x57, 0x62));
				g.FillRectangle(&brush, rectBounds.left, rectBounds.top, rectBounds.Width()+1, rectBounds.Height()+1);
			}
			else
				if (m_Hover)
				{
					SolidBrush brush1(Color(0x40, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush1, rectBounds.left, rectBounds.top+1, rectBounds.Width(), rectBounds.Height()/2+1);

					SolidBrush brush2(Color(0x28, 0xA0, 0xAF, 0xC3));
					g.FillRectangle(&brush2, rectBounds.left, rectBounds.top+rectBounds.Height()/2+2, rectBounds.Width(), rectBounds.Height()/2-1);
				}

			g.SetSmoothingMode(SmoothingModeAntiAlias);
			GraphicsPath path;

			if (!Selected)
			{
				CreateRoundRectangle(rectBounds, 1, path);

				Pen pen(Color(0x80, 0xFF, 0xFF, 0xFF));
				g.DrawPath(&pen, &path);
			}

			// Outer Border
			rectBounds.InflateRect(1, 1);
			CreateRoundRectangle(rectBounds, 2, path);

			Pen pen(Color(0x70, 0x50, 0x57, 0x62));
			g.DrawPath(&pen, &path);
		}

		dc.SetTextColor(m_Hover ? 0x404040 : 0x333333);
	}
	else
	{
		if ((Selected) || (m_Hover))
		{
			COLORREF c1 = Selected ? 0x000000 : GetSysColor(COLOR_3DHIGHLIGHT);
			COLORREF c2 = Selected ? GetSysColor(COLOR_3DSHADOW) : GetSysColor(COLOR_3DFACE);
			COLORREF c3 = Selected ? GetSysColor(COLOR_3DFACE) : GetSysColor(COLOR_3DSHADOW);
			COLORREF c4 = Selected ? GetSysColor(COLOR_3DHIGHLIGHT) : 0x000000;

			CRect rectBorder(rect);
			dc.Draw3dRect(rectBorder, c1, c4);
			rectBorder.DeflateRect(1, 1);
			dc.Draw3dRect(rectBorder, c2, c3);
		}

		if (Focused)
		{
			CRect rectFocus(rect);
			rectFocus.DeflateRect(2, 2);
			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rectFocus);
		}

		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	}

	// Content
	CRect rectText(rect);
	rectText.DeflateRect(BORDER+2, BORDER);
	if (Selected)
		rectText.OffsetRect(1, 1);

	// Icon
	if ((p_Icons) && (m_IconID!=-1))
	{
		CAfxDrawState ds;
		p_Icons->PrepareDrawImage(ds);

		CPoint pt(rectText.left, (rect.Height()-m_IconSize)/2+(Selected ? 2 : 1));
		p_Icons->Draw(&dc, pt.x, pt.y, m_IconID);

		if (m_OverlayID!=-1)
			p_Icons->Draw(&dc, pt.x+5, pt.y-(m_IconSize==32 ? 2 : 3), m_OverlayID);

		p_Icons->EndDrawImage(ds);

		rectText.left += m_IconSize+BORDER;
	}

	// Text
	HFONT hOldFont = (HFONT)dc.SelectObject(Themed ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
	dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	dc.SelectObject(hOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CTaskButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CButton::OnMouseMove(nFlags, point);

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

void CTaskButton::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	Invalidate();

	CButton::OnMouseLeave();
}

void CTaskButton::OnMouseHover(UINT nFlags, CPoint point)
{
	if (!m_TooltipHeader.IsEmpty())
		if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
		{
			ClientToScreen(&point);
			m_TooltipCtrl.Track(point, NULL, m_TooltipHint.IsEmpty() ? _T("") : m_TooltipHeader, m_TooltipHint.IsEmpty() ? m_TooltipHeader : m_TooltipHint);
		}
		else
		{
			m_TooltipCtrl.Deactivate();
		}
}

void CTaskButton::OnSetFocus(CWnd* pOldWnd)
{
	CButton::OnSetFocus(pOldWnd);
	Invalidate();
}


void CTaskButton::OnKillFocus(CWnd* pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
	Invalidate();
}
