
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskButton
//

#define BORDER     4

BOOL CTaskButton::Create(CWnd* pParentWnd, UINT nID, const CString& Caption, const CString& Hint, CIcons* pButtonIcons, CIcons* pTooltipIcons, INT IconID, BOOL ForceSmall, BOOL HideIcon)
{
	m_Caption = Caption;
	m_Hint = Hint;
	p_ButtonIcons = pButtonIcons;
	p_TooltipIcons = pTooltipIcons;
	m_IconID = IconID;
	m_ForceSmall = ForceSmall;
	m_HideIcon = HideIcon;

	return CHoverButton::Create(Caption, pParentWnd, nID);
}

void CTaskButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect(lpDrawItemStruct->rcItem);

	CDC dc;
	dc.Attach(CreateCompatibleDC(lpDrawItemStruct->hDC));
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.Attach(CreateCompatibleBitmap(lpDrawItemStruct->hDC, rect.Width(), rect.Height()));
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// State
	const BOOL Disabled = (lpDrawItemStruct->itemState & ODS_DISABLED);
	const BOOL Focused = (lpDrawItemStruct->itemState & ODS_FOCUS);
	const BOOL Selected = (lpDrawItemStruct->itemState & ODS_SELECTED);

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)lpDrawItemStruct->hwndItem));

	// Button
	BOOL Themed = IsCtrlThemed();

	DrawLightButtonBackground(dc, rect, Themed, Focused, Selected, m_Hover);

	// Content
	CRect rectText(rect);
	rectText.DeflateRect(BORDER+2, BORDER);
	if (Selected)
		rectText.OffsetRect(1, 1);

	// Icon
	if (p_ButtonIcons && (!m_HideIcon || m_Small))
	{
		const INT IconSize = p_ButtonIcons->GetIconSize();

		p_ButtonIcons->Draw(dc, rectText.left, (rect.Height()-IconSize)/2+(Selected ? 1 : 0), m_IconID, m_Hover, Disabled);
		rectText.left += IconSize+BORDER;
	}

	// Text
	if (!m_Small)
	{
		dc.SetTextColor(Themed ? m_Hover ? 0x333333 : 0x404040 : GetSysColor(COLOR_WINDOWTEXT));

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);
		dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX);
		dc.SelectObject(pOldFont);
	}

	BitBlt(lpDrawItemStruct->hDC, 0, 0, rect.Width(), rect.Height(), dc.m_hDC, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
	DeleteDC(dc.Detach());
	DeleteObject(MemBitmap.Detach());
}

void CTaskButton::SetIconID(INT IconID)
{
	m_IconID = IconID;

	Invalidate();
}

INT CTaskButton::GetPreferredWidth(BOOL Small)
{
	m_Small = Small | m_ForceSmall;

	INT Width = 2*(BORDER+2);

	if (p_ButtonIcons && (!m_HideIcon || m_Small))
	{
		Width += p_ButtonIcons->GetIconSize();

		if (!m_Small)
			Width += BORDER;
	}

	if (!m_Small)
		Width += LFGetApp()->m_DefaultFont.GetTextExtent(m_Caption).cx;

	return Width;
}


BEGIN_MESSAGE_MAP(CTaskButton, CHoverButton)
	ON_NOTIFY_REFLECT(REQUEST_TOOLTIP_DATA, OnRequestTooltipData)
END_MESSAGE_MAP()

void CTaskButton::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	wcscpy_s(pTooltipData->Caption, 256, m_Caption);
	wcscpy_s(pTooltipData->Hint, 4096, m_Hint);
	pTooltipData->hIcon = p_TooltipIcons->ExtractIcon(m_IconID, IsCtrlThemed());

	*pResult = TRUE;
}
