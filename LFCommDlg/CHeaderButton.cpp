
// CHeaderButton.cpp: Implementierung der Klasse CHeaderButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CHeaderButton
//

#define BORDER     4

CHeaderButton::CHeaderButton()
	: CButton()
{
	m_Hover = FALSE;
	m_Value = _T("?");
	m_ShowDropdown = TRUE;
}

BOOL CHeaderButton::Create(CWnd* pParentWnd, UINT nID, CString Caption, CString Hint)
{
	m_Caption = Caption;
	m_Hint = Hint;

	CRect rect;
	rect.SetRectEmpty();
	return CButton::Create(_T(""), WS_VISIBLE | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, rect, pParentWnd, nID);
}

BOOL CHeaderButton::PreTranslateMessage(MSG* pMsg)
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

void CHeaderButton::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}

void CHeaderButton::SetValue(CString Value, BOOL ShowDropdown, BOOL Repaint)
{
	m_Value = Value;
	m_ShowDropdown = ShowDropdown;

	if (Repaint)
		GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CHeaderButton::GetPreferredSize(CSize& sz, UINT& CaptionWidth)
{
	CDC* pDC = GetDC();
	HFONT hOldFont = (HFONT)pDC->SelectObject(IsCtrlThemed() ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
	sz = pDC->GetTextExtent(m_Value.IsEmpty() ? _T("Wy") : m_Value);
	m_CaptionWidth = CaptionWidth = m_Caption.IsEmpty() ? 0 : pDC->GetTextExtent(m_Caption+_T(":")).cx;
	pDC->SelectObject(hOldFont);
	ReleaseDC(pDC);

	sz.cx += m_ShowDropdown ? 3*BORDER+14 : 2*BORDER+5;
	sz.cy += 2*BORDER;
}

void CHeaderButton::GetCaption(CString& Caption, UINT& CaptionWidth)
{
	Caption = m_Caption;
	if (!Caption.IsEmpty())
		Caption += _T(":");

	CaptionWidth = m_CaptionWidth;
}


BEGIN_MESSAGE_MAP(CHeaderButton, CButton)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

INT CHeaderButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

BOOL CHeaderButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHeaderButton::OnPaint()
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

	CRect rectText(rect);
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
	}
	else
	{
		if ((Selected) || (m_Hover))
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));

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
	}

	// Content
	rectText.DeflateRect(BORDER+2, BORDER);
	if (Selected)
		rectText.OffsetRect(1, 1);

	// Text
	COLORREF clrText = Themed ? m_Hover ? 0xCC6633 : 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT);
	dc.SetTextColor(clrText);

	HFONT hOldFont = (HFONT)dc.SelectObject(Themed ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
	dc.DrawText(m_Value, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	dc.SelectObject(hOldFont);

	// Icon
	if (m_ShowDropdown)
	{
		CPen pen;
		pen.CreatePen(PS_SOLID, 1, clrText);
		CPen* pOldPen = dc.SelectObject(&pen);

		INT Row = rectText.top+(rectText.Height()-4)/2;
		for (UINT a=0; a<4; a++)
		{
			dc.MoveTo(rectText.right-a-2, Row);
			dc.LineTo(rectText.right+a-9, Row);

			Row++;
		}

		dc.SelectObject(pOldPen);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CHeaderButton::OnMouseMove(UINT nFlags, CPoint point)
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

void CHeaderButton::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	Invalidate();

	CButton::OnMouseLeave();
}

void CHeaderButton::OnMouseHover(UINT nFlags, CPoint point)
{
	if (!m_Hint.IsEmpty())
		if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
		{
			ClientToScreen(&point);
			m_TooltipCtrl.Track(point, NULL, m_Caption, m_Hint);
		}
		else
		{
			m_TooltipCtrl.Deactivate();
		}
}

void CHeaderButton::OnSetFocus(CWnd* pOldWnd)
{
	CButton::OnSetFocus(pOldWnd);
	Invalidate();
}


void CHeaderButton::OnKillFocus(CWnd* pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
	Invalidate();
}

void CHeaderButton::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	if (m_ShowDropdown)
	{
		GetOwner()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
	else
	{
		CButton::OnContextMenu(pWnd, pos);
	}
}
