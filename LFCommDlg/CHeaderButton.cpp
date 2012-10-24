
// CHeaderButton.cpp: Implementierung der Klasse CHeaderButton
//

#include "stdafx.h"
#include "CGlassWindow.h"
#include "CHeaderButton.h"
#include "LFCommDlg.h"


// CHeaderButton
//

#define BORDER          4

CHeaderButton::CHeaderButton()
	: CButton()
{
	m_Hover = FALSE;
	m_Value = _T("?");
}

BOOL CHeaderButton::Create(CString Caption, CString Hint, CWnd* pParentWnd, UINT nID)
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

void CHeaderButton::SetValue(CString Value, BOOL Repaint)
{
	m_Value = Value;

	if (Repaint)
		GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CHeaderButton::GetPreferredSize(CSize& sz, UINT& CaptionWidth)
{
	CDC* dc = GetDC();
	HFONT hOldFont = IsCtrlThemed() ? (HFONT)dc->SelectObject(LFGetApp()->m_DefaultFont.m_hObject) : (HFONT)dc->SelectStockObject(DEFAULT_GUI_FONT);
	sz = dc->GetTextExtent(m_Value.IsEmpty() ? _T("Wy") : m_Value);
	m_CaptionWidth = CaptionWidth = dc->GetTextExtent(m_Caption+_T(":")).cx;
	dc->SelectObject(hOldFont);
	ReleaseDC(dc);

	sz.cx += 3*BORDER+2+7;
	sz.cy += 2*BORDER;
}

void CHeaderButton::GetCaption(CString& Caption, UINT& CaptionWidth)
{
	Caption = m_Caption+_T(":");
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

	// Button
	CRect rectText(rect);
	COLORREF clrText = 0x000000;
	if (IsCtrlThemed())
	{
		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		if ((Focused) || (Selected) || (m_Hover))
		{
			// Outer border
			CRect rectBounds(rect);

			if (LFGetApp()->OSVersion==OS_Eight)
			{
				if ((Focused) || (Selected) || (m_Hover))
				{
					COLORREF colBorder = Selected ? 0xDAA026 : m_Hover ? 0xEDC093 : 0xDAA026;
					COLORREF colInner = Selected ? 0xF0E1C3 : m_Hover ? 0xF8F0E1 : 0xFFFFFF;

					CRect rectBounds(rect);
					dc.Draw3dRect(rectBounds, colBorder, colBorder);

					if (Selected || m_Hover)
					{
						rectBounds.DeflateRect(1, 1);
						dc.FillSolidRect(rectBounds, colInner);
					}
				}
			}
			else
			{
				rectBounds.right--;
				rectBounds.bottom--;

				GraphicsPath path;
				CreateRoundRectangle(rectBounds, 2, path);

				Pen pen(Color(0x80, 0xB8, 0xC8, 0xD8));
				g.DrawPath(&pen, &path);

				// Inner border
				rectBounds.DeflateRect(1, 1);
				CreateRoundRectangle(rectBounds, 1, path);

				g.SetSmoothingMode(SmoothingModeDefault);

				if (Selected)
				{
					LinearGradientBrush brush1(Point(rectBounds.left, rectBounds.top), Point(rectBounds.left, rectBounds.top+3), Color(0x24, 0x50, 0x57, 0x62), Color(0x0C, 0x50, 0x58, 0x62));
					g.FillRectangle(&brush1, rectBounds.left, rectBounds.top, rectBounds.Width()+1, 3);

					SolidBrush brush2(Color(0x08, 0x50, 0x58, 0x62));
					g.FillRectangle(&brush2, rectBounds.left, rectBounds.top+3, rectBounds.Width()+1, rectBounds.Height()-2);
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

				if (!Selected)
				{
					pen.SetColor(Color(0x80, 0xFF, 0xFF, 0xFF));
					g.DrawPath(&pen, &path);
				}

				clrText = m_Hover ? 0xFF9933 : 0xCC6600;
			}
		}

		rectText.DeflateRect(BORDER, BORDER);
		if (Selected)
			rectText.OffsetRect(1, 1);

		dc.SetTextColor(clrText);
		dc.DrawText(m_Value, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

		dc.SelectObject(pOldFont);
	}
	else
	{
		COLORREF c1 = GetSysColor(COLOR_3DHIGHLIGHT);
		COLORREF c2 = GetSysColor(COLOR_3DFACE);
		COLORREF c3 = GetSysColor(COLOR_3DSHADOW);
		COLORREF c4 = 0x000000;

		if ((Selected) || (m_Hover))
		{
			dc.FillSolidRect(rect, c2);

			if (Selected)
			{
				std::swap(c1, c4);
				std::swap(c2, c3);
			}

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

		rectText.DeflateRect(BORDER+2, BORDER);
		if (Selected)
			rectText.OffsetRect(1, 1);

		HFONT hOldFont = (HFONT)dc.SelectStockObject(DEFAULT_GUI_FONT);

		clrText = GetSysColor(COLOR_WINDOWTEXT);

		dc.SetTextColor(clrText);
		dc.DrawText(m_Value, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

		dc.SelectObject(hOldFont);
	}

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, clrText);
	CPen* pOldPen = dc.SelectObject(&pen);

	INT Row = rectText.top+(rectText.Height()-4)/2;
	for (UINT a=0; a<4; a++)
	{
		dc.MoveTo(rectText.right-a-1, Row);
		dc.LineTo(rectText.right+a-8, Row);

		Row++;
	}

	dc.SelectObject(pOldPen);

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
			m_TooltipCtrl.Track(point, NULL, NULL, m_Caption, m_Hint);
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

void CHeaderButton::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
	GetOwner()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}
