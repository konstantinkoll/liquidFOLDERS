
// CStoreButton.cpp: Implementierung der Klasse CStoreButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


void DDX_StoreButton(CDataExchange* pDX, INT nIDC, CStoreButton &rControl, UINT SourceType)
{
	DDX_Control(pDX, nIDC, rControl);

	rControl.ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW);
	rControl.SetStoreType(SourceType);
}


// CStoreButton
//

#define BORDER     12

CStoreButton::CStoreButton()
	: CButton()
{
	p_Icons = NULL;
	m_IconSize = 0;
	m_IconID = -1;
	m_Hover = FALSE;
}

void CStoreButton::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}

void CStoreButton::SetStoreType(UINT StoreType)
{
	LFGetSourceName(m_Caption, 256, StoreType, FALSE);

	CRect rectClient;
	GetClientRect(rectClient);

	INT h = rectClient.Height()-2*BORDER;

	m_IconSize = (h>=128) ? 128 : (h>=96) ? 96 : 48;
	p_Icons = (m_IconSize==128) ? &LFGetApp()->m_CoreImageListJumbo : (m_IconSize==96) ? &LFGetApp()->m_CoreImageListHuge : &LFGetApp()->m_CoreImageListExtraLarge;
	m_IconID = StoreType-1;

	Invalidate();
}


BEGIN_MESSAGE_MAP(CStoreButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CStoreButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CStoreButton::OnPaint()
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
	HBRUSH hBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (hBrush)
		FillRect(dc, rect, hBrush);

	// Button
	BOOL Themed = IsCtrlThemed();
	if (Themed)
	{
		Graphics g(dc);

		CRect rectBounds(rect);
		g.SetSmoothingMode(SmoothingModeNone);

		// Inner border
		rectBounds.DeflateRect(2, 2);

		if (Selected)
		{
		/*	dc.FillSolidRect(rectBounds, Selected ? 0xF0D197 : 0xFDE2B0);


			LinearGradientBrush brush5(Point(0, rectBounds.top-1), Point(0, (rectBounds.top+rectBounds.bottom)/2), Color(0xC0, 0xFF, 0xFF, 0xFF), Color(0x80, 0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush5, rectBounds.left, rectBounds.top, rectBounds.Width(), rectBounds.Height()/2);

			if (Selected)
			{
			}*/
//						COLORREF colBorder = Hover ? 0xEDC093 : 0xDAA026;
//						COLORREF colInner = Hover ? 0xF8F0E1 : 0xF0E1C3;

			dc.FillSolidRect(rectBounds, 0xF6E4C2);

			LinearGradientBrush brush3(Point(0, rectBounds.top), Point(0, rectBounds.top+4), Color(0x20, 0x00, 0x00, 0x00), Color(0x00, 0x00, 0x00, 0x00));
			g.FillRectangle(&brush3, rectBounds.left, rectBounds.top, rectBounds.Width(), 4);
		}
		else
		{
			dc.FillSolidRect(rectBounds, m_Hover ? 0xEFECEC : 0xF7F4F4);

			LinearGradientBrush brush3(Point(0, rectBounds.top-1), Point(0, (rectBounds.top+rectBounds.bottom)/2), Color(0xFF, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush3, rectBounds.left, rectBounds.top, rectBounds.Width(), rectBounds.Height()/2);

			LinearGradientBrush brush4(Point(0, rectBounds.bottom-4), Point(0, rectBounds.bottom+1), Color(0x00, 0x00, 0x00, 0x00), Color(m_Hover ? 0x18 : 0x10, 0x00, 0x00, 0x00));
			g.FillRectangle(&brush4, rectBounds.left, rectBounds.bottom-3, rectBounds.Width(), 3);
		}

		g.SetSmoothingMode(SmoothingModeAntiAlias);

		rectBounds.left--;
		rectBounds.top--;

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);

		if (Focused)
		{
			Pen pen(Color(0x32, 0x75, 0xA7));
			g.DrawPath(&pen, &path);

			rectBounds.DeflateRect(1, 1);
			CreateRoundRectangle(rectBounds, 2, path);

			Pen pen2(Color(0xC0, 0x48, 0xD8, 0xFB));
			g.DrawPath(&pen2, &path);

			rectBounds.InflateRect(1, 1);
		}
		else
			if (m_Hover)
			{
			Pen pen(Color(0xAC, 0xAF, 0xB4));
			g.DrawPath(&pen, &path);
			}
			else
		{
			Pen pen(Color(0xBE, 0xBE, 0xBE));
			g.DrawPath(&pen, &path);
		}
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);

		if (Focused)
		{
			CRect rectFocus(rect);
			rectFocus.DeflateRect(2, 2);
			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rectFocus);
		}
	}

	// Content
	dc.SetTextColor(IsWindowEnabled() ? Themed ? m_Hover || Focused | Selected ? 0x000000 : 0x404040 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_GRAYTEXT));

	CRect rectText(rect);
	rectText.DeflateRect(BORDER+1, BORDER);
	if (Selected)
		rectText.OffsetRect(1, 1);

	// Icon
	if ((p_Icons) && (m_IconID!=-1))
	{
		CPoint pt(rectText.left, (rect.Height()-m_IconSize)/2+(Selected ? 1 : 0));
		p_Icons->DrawEx(&dc, m_IconID, pt, CSize(m_IconSize, m_IconSize), CLR_NONE, 0xFFFFFF, IsWindowEnabled() ? ILD_TRANSPARENT : ILD_BLEND25);

		rectText.left += m_IconSize+BORDER;
	}

	// Text
	CString Hint;
	GetWindowText(Hint);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_LargeFont);

	INT HeightCaption = dc.GetTextExtent(m_Caption).cy;
	HeightCaption += HeightCaption/2;

	dc.SelectObject(&LFGetApp()->m_DefaultFont);

	CRect rectHint(rectText);
	dc.DrawText(Hint, rectHint, DT_CALCRECT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	rectText.top += (rectText.Height()-HeightCaption-rectHint.Height())/2;

	dc.SelectObject(&LFGetApp()->m_LargeFont);
	dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);
	rectText.top += HeightCaption;

	dc.SelectObject(&LFGetApp()->m_DefaultFont);
	dc.DrawText(Hint, rectText, DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
	DeleteDC(dc.Detach());
	DeleteObject(MemBitmap.Detach());
}

void CStoreButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CButton::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
}

LRESULT CStoreButton::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_Hover = FALSE;
	Invalidate();

	return NULL;
}

void CStoreButton::OnSetFocus(CWnd* pOldWnd)
{
	CButton::OnSetFocus(pOldWnd);
	Invalidate();
}


void CStoreButton::OnKillFocus(CWnd* pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);
	Invalidate();
}
