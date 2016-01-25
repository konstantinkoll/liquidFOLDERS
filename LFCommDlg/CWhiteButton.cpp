
// CWhiteButton.cpp: Implementierung der Klasse CWhiteButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CWhiteButton
//

CWhiteButton::CWhiteButton()
	: CHoverButton()
{
}

void CWhiteButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect(lpDrawItemStruct->rcItem);

	CDC dc;
	dc.Attach(CreateCompatibleDC(lpDrawItemStruct->hDC));
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.Attach(CreateCompatibleBitmap(lpDrawItemStruct->hDC, rect.Width(), rect.Height()));
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// State
	BOOL Focused = lpDrawItemStruct->itemState & ODS_FOCUS;
	BOOL Selected = lpDrawItemStruct->itemState & ODS_SELECTED;

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)lpDrawItemStruct->hwndItem));

	Graphics g(dc);
	DrawWhiteButtonBorder(g, rect);

	// Button
	DrawWhiteButtonBackground(dc, rect, IsCtrlThemed(), Focused, Selected, m_Hover, lpDrawItemStruct->itemState & ODS_DISABLED);
	DrawWhiteButtonForeground(dc, lpDrawItemStruct, Selected);

	BitBlt(lpDrawItemStruct->hDC, 0, 0, rect.Width(), rect.Height(), dc.m_hDC, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
	DeleteDC(dc.Detach());
	DeleteObject(MemBitmap.Detach());
}
