
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "CTaskButton.h"
#include "LFCommDlg.h"


// CTaskButton
//

#define BORDER          4

CTaskButton::CTaskButton()
	: CButton()
{
	m_Hover = FALSE;
}

BOOL CTaskButton::Create(CString Caption, CString Tooltip, CMFCToolBarImages* Icons, int IconID, CWnd* pParentWnd, UINT nID)
{
	m_Caption = Caption;
	m_Tooltip = Tooltip;
	m_Icons = Icons;
	m_IconID = IconID;

	CRect rect;
	rect.SetRectEmpty();
	return CButton::Create(Caption, WS_VISIBLE | WS_TABSTOP | WS_GROUP, rect, pParentWnd, nID);
}

BOOL CTaskButton::PreTranslateMessage(MSG* pMsg)
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
		m_TooltipCtrl.Hide();
		break;
	}

	return CButton::PreTranslateMessage(pMsg);
}

int CTaskButton::GetPreferredWidth()
{
	int l = 2*(BORDER+2)+1;

	if ((m_Icons) && (m_IconID!=-1))
		l += 16+(m_Caption.IsEmpty() ? 0 : BORDER);

	if (!m_Caption.IsEmpty())
	{
		CSize sz;

		CDC* dc = GetDC();
		CFont* pOldFont = dc->SelectObject(&((LFApplication*)AfxGetApp())->m_DefaultFont);
		sz = dc->GetTextExtent(m_Caption);
		dc->SelectObject(pOldFont);
		ReleaseDC(dc);

		l += sz.cx;
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
END_MESSAGE_MAP()

int CTaskButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// State
	BOOL Focused = (GetState() & 8);
	BOOL Selected = (GetState() & 4);

	// Background
	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	Graphics g(dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_DefaultFont);

	OSVERSIONINFO i = ((LFApplication*)AfxGetApp())->osInfo;
	if ((i.dwMajorVersion>=6) && (i.dwMinorVersion!=0))
	{
		if ((Focused) || (Selected) || (m_Hover))
		{
			// Outer border
			CRect rectBounds(rect);
			rectBounds.right--;
			rectBounds.bottom--;

			GraphicsPath path;
			CreateRoundRectangle(rectBounds, 2, path);
	
			Pen pen(Color(0x70, 0x50, 0x57, 0x62));
			g.DrawPath(&pen, &path);

			// Inner border
			rectBounds.DeflateRect(1, 1);
			CreateRoundRectangle(rectBounds, 1, path);

			g.SetSmoothingMode(SmoothingModeDefault);

			if (Selected)
			{
				SolidBrush brush(Color(0x24+1, 0x50, 0x57, 0x62));
				g.FillRectangle(&brush, rectBounds.left, rectBounds.top, rectBounds.Width()+1, rectBounds.Height()+1);
			}
			else
				if (m_Hover)
				{
					SolidBrush brush1(Color(0x40, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush1, rectBounds.left, rectBounds.top+1, rectBounds.Width(), rectBounds.Height()/2+1);

					SolidBrush brush2(Color(0x40, 0xA0, 0xAF, 0xC3));
					g.FillRectangle(&brush2, rectBounds.left, rectBounds.top+rectBounds.Height()/2+2, rectBounds.Width(), rectBounds.Height()/2-1);
				}

			g.SetSmoothingMode(SmoothingModeAntiAlias);

			if (!Selected)
			{
				pen.SetColor(Color(0x80, 0xFF, 0xFF, 0xFF));
				g.DrawPath(&pen, &path);
			}
		}

		CRect rectText(rect);
		rectText.DeflateRect(BORDER+2, BORDER);
		if (Selected)
			rectText.OffsetRect(1, 1);

		if ((m_Icons) && (m_IconID!=-1))
		{
			CAfxDrawState ds;
			m_Icons->PrepareDrawImage(ds);
			m_Icons->Draw(&dc, rectText.left, (rect.Height()-rectText.Height())/2+(Selected ? 1 : 0), m_IconID);
			m_Icons->EndDrawImage(ds);

			rectText.left += 16+BORDER;
		}

		dc.SetTextColor(0x5B391E);
		dc.DrawText(m_Caption, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		if ((Focused) || (Selected) || (m_Hover))
		{
			// Outer border
			CRect rectBounds(rect);
			rectBounds.right--;
			rectBounds.bottom--;

			GraphicsPath path;
			CreateRoundRectangle(rectBounds, 4, path);
	
			Pen pen(Color(0x58, 0x00, 0x00, 0x00));
			g.DrawPath(&pen, &path);

			// Inner border
			rectBounds.DeflateRect(1, 1);
			CreateRoundRectangle(rectBounds, 2, path);

			if ((m_Hover) || (Selected))
			{
				// Shine
				Color c1;
				Color c2;
				if (Selected)
				{
					c1 = Color(0x20, 0x00, 0x00, 0x00);
					c2 = Color(0x40, 0x00, 0x00, 0x00);
				}
				else
				{
					c1 = Color(0x40, 0xFF, 0xFF, 0xFF);
					c2 = Color(0x00, 0xFF, 0xFF, 0xFF);
				}

				LinearGradientBrush brush(Point(0, rectBounds.top), Point(0, rectBounds.bottom), c1, c2);
				g.FillRectangle(&brush, rectBounds.left, rectBounds.top, rectBounds.Width(), rectBounds.Height());
			}

			pen.SetColor(Color(0x58, 0xFF, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
		}

		CRect rectText(rect);
		rectText.DeflateRect(BORDER+2, BORDER);
		if (Selected)
			rectText.OffsetRect(1, 1);

		if ((m_Icons) && (m_IconID!=-1))
		{
			CAfxDrawState ds;
			m_Icons->PrepareDrawImage(ds);
			m_Icons->Draw(&dc, rectText.left, (rect.Height()-rectText.Height())/2+(Selected ? 1 : 0), m_IconID);
			m_Icons->EndDrawImage(ds);

			rectText.left += 16+BORDER;
		}

		rectText.OffsetRect(1, 1);
		dc.SetTextColor(0x000000);
		dc.DrawText(m_Caption, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

		rectText.OffsetRect(-1, -1);
		dc.SetTextColor(0xFFFFFF);
		dc.DrawText(m_Caption, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
	dc.SelectObject(pOldFont);
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
		tme.dwHoverTime = HOVER_DEFAULT;
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
	if (!m_Tooltip.IsEmpty())
	{
		if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
		{
			ClientToScreen(&point);
			m_TooltipCtrl.Track(point, m_Tooltip);
		}
		else
		{
			m_TooltipCtrl.Deactivate();
		}

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}
