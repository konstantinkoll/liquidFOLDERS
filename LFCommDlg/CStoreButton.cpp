
// CStoreButton.cpp: Implementierung der Klasse CStoreButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


void DDX_StoreButton(CDataExchange* pDX, int nIDC, CStoreButton &rControl, UINT SourceType)
{
	DDX_Control(pDX, nIDC, rControl);

	rControl.ModifyStyle(0, BS_OWNERDRAW);
	rControl.SetStoreType(SourceType);
}


// CStoreButton
//

#define BORDER     12

CStoreButton::CStoreButton()
	: CButton()
{
	p_App = LFGetApp();
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
	LFGetSourceName(m_Caption, StoreType, false);

	CRect rectClient;
	GetClientRect(rectClient);

	INT h = rectClient.Height()-2*BORDER;

	m_IconSize = (h>=128) ? 128 : (h>=96) ? 96 : 48;
	p_Icons = (m_IconSize==128) ? &p_App->m_CoreImageListJumbo : (m_IconSize==96) ? &p_App->m_CoreImageListHuge : &p_App->m_CoreImageListExtraLarge;
	m_IconID = StoreType;

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
	BOOL Themed = IsCtrlThemed();
	if (Themed)
	{
		Graphics g(dc);
		g.SetSmoothingMode(SmoothingModeNone);

		CRect rectBounds(rect);
		rectBounds.right--;
		rectBounds.bottom--;

		// Inner border
		rectBounds.DeflateRect(1, 1);

		if (Selected)
		{
			dc.FillSolidRect(rect, 0xEEE9E5);
		}
		else
			if (Focused)
			{
				LinearGradientBrush brush(Point(0, rectBounds.top+rectBounds.Height()*3/5), Point(0, rectBounds.bottom), Color(0xFF, 0xFF, 0xFF), Color(0xE5, 0xE9, 0xEE));
				g.FillRectangle(&brush, rectBounds.left, rectBounds.top+rectBounds.Height()*3/5+1, rectBounds.Width()+1, rectBounds.Height()+1);
			}
			else
			{
				dc.FillSolidRect(rect, 0xFEFDFD);
			}

		g.SetSmoothingMode(SmoothingModeAntiAlias);
		GraphicsPath path;

		if (!Selected)
		{
			CreateRoundRectangle(rectBounds, 1, path);

			Pen pen(Color(0x80, 0xFF, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
		}

		rectBounds.InflateRect(1, 1);
		CreateRoundRectangle(rectBounds, 2, path);

		if (m_Hover || Focused || Selected)
		{
			Pen pen(Color(0xB1, 0xB4, 0xB9));
			g.DrawPath(&pen, &path);
		}
		else
		{
			Pen pen(Color(0xED, 0xED, 0xEE));
			g.DrawPath(&pen, &path);
		}
	}
	else
	{
		if (Selected)
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));

		if (Focused)
		{
			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}

	// Content
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

	CFont* pOldFont = dc.SelectObject(&p_App->m_LargeFont);

	INT HeightCaption = dc.GetTextExtent(m_Caption).cy;
	HeightCaption += HeightCaption/2;

	dc.SelectObject(&p_App->m_DefaultFont);

	CRect rectHint(rectText);
	dc.DrawText(Hint, rectHint, DT_CALCRECT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	rectText.top += (rectText.Height()-HeightCaption-rectHint.Height())/2;

	dc.SetTextColor(IsWindowEnabled() ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_GRAYTEXT));
	dc.SelectObject(&p_App->m_LargeFont);
	dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);
	rectText.top += HeightCaption;

	dc.SetTextColor(IsWindowEnabled() ? Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_GRAYTEXT));
	dc.SelectObject(&p_App->m_DefaultFont);
	dc.DrawText(Hint, rectText, DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
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
