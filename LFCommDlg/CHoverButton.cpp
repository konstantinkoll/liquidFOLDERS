
// CHoverButton.cpp: Implementierung der Klasse CHoverButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CHoverButton
//

CHoverButton::CHoverButton()
	: CButton()
{
	CONSTRUCTOR_TOOLTIP()

	m_DrawBorder = FALSE;
}

BOOL CHoverButton::Create(LPCTSTR lpszCaption, CWnd* pParentWnd, UINT nID)
{
	m_DrawBorder = TRUE;

	return CButton::Create(lpszCaption, WS_VISIBLE | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, CRect(0, 0, 0, 0), pParentWnd, nID);
}

void CHoverButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect(lpDrawItemStruct->rcItem);

	CDC dc;
	dc.Attach(CreateCompatibleDC(lpDrawItemStruct->hDC));
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.Attach(CreateCompatibleBitmap(lpDrawItemStruct->hDC, rect.Width(), rect.Height()));
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	Graphics g(dc);

	// State
	const BOOL Disabled = (lpDrawItemStruct->itemState & ODS_DISABLED);
	const BOOL Focused = (lpDrawItemStruct->itemState & ODS_FOCUS);
	const BOOL Selected = (lpDrawItemStruct->itemState & ODS_SELECTED);

	// Background
	FillRect(dc, rect, (HBRUSH)GetOwner()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)lpDrawItemStruct->hwndItem));

	// Button
	const BOOL Themed = IsCtrlThemed();

	if (Themed && m_DrawBorder)
		DrawWhiteButtonBorder(g, rect);

	DrawWhiteButtonBackground(dc, g, rect, Themed, Focused, Selected, m_HoverItem>=0, Disabled);

	// Content
	NM_DRAWBUTTONFOREGROUND tag;
	ZeroMemory(&tag, sizeof(tag));

	tag.hdr.code = REQUEST_DRAWBUTTONFOREGROUND;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.lpDrawItemStruct = lpDrawItemStruct;
	tag.pDC = &dc;
	tag.Hover = (m_HoverItem>=0);
	tag.Themed = Themed;

	if (Selected)
		::OffsetRect(&lpDrawItemStruct->rcItem, 1, 1);

	if (!GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag)))
		DrawWhiteButtonForeground(dc, lpDrawItemStruct, (GetParent()->SendMessage(WM_QUERYUISTATE) & UISF_HIDEACCEL)==0);

	// Blit to screen
	BitBlt(lpDrawItemStruct->hDC, 0, 0, rect.Width(), rect.Height(), dc.m_hDC, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
	DeleteDC(dc.Detach());
	DeleteObject(MemBitmap.Detach());
}

INT CHoverButton::ItemAtPosition(CPoint /*point*/) const
{
	return 0;
}

CPoint CHoverButton::PointAtPosition(CPoint /*point*/) const
{
	return CPoint(-1, -1);
}

LPCVOID CHoverButton::PtrAtPosition(CPoint /*point*/) const
{
	return NULL;
}

void CHoverButton::InvalidateItem(INT /*Index*/)
{
	Invalidate();
}

void CHoverButton::InvalidatePoint(const CPoint& /*point*/)
{
	Invalidate();
}

void CHoverButton::InvalidatePtr(LPCVOID /*Ptr*/)
{
	Invalidate();
}

void CHoverButton::ShowTooltip(const CPoint& point)
{
	NM_TOOLTIPDATA tag;
	ZeroMemory(&tag, sizeof(tag));

	tag.hdr.code = REQUEST_TOOLTIP_DATA;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();

	if (GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag)))
		LFGetApp()->ShowTooltip(this, point, tag.Caption, tag.Hint, tag.hIcon, tag.hBitmap);
}


IMPLEMENT_TOOLTIP_NOWHEEL(CHoverButton, CButton)

BEGIN_TOOLTIP_MAP(CHoverButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CHoverButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHoverButton::OnSetFocus(CWnd* pOldWnd)
{
	CButton::OnSetFocus(pOldWnd);

	Invalidate();
}

void CHoverButton::OnKillFocus(CWnd* pNewWnd)
{
	CButton::OnKillFocus(pNewWnd);

	Invalidate();
}
