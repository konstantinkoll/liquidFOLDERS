
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

	// Background
	HBRUSH hBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (hBrush)
		FillRect(dc, rect, hBrush);

	// Button
	BOOL Themed = IsCtrlThemed();
	BOOL Selected = (GetState() & 4);

	DrawWhiteButtonBackground(dc, rect, Themed, GetState() & 8, Selected, m_Hover);

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
	if (!IsWindowEnabled())
		dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));

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
