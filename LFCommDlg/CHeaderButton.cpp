
// CHeaderButton.cpp: Implementierung der Klasse CHeaderButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CHeaderButton
//

#define MARGIN     4

CHeaderButton::CHeaderButton()
	: CHoverButton()
{
	m_Hover = m_ShowDropdown = FALSE;
}

BOOL CHeaderButton::Create(CWnd* pParentWnd, UINT nID, const CString& Caption, const CString& Hint)
{
	m_Caption = m_Value = Caption;
	m_Hint = Hint;

	return CHoverButton::Create(Caption, pParentWnd, nID);
}

void CHeaderButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	const BOOL Themed = IsCtrlThemed();

	DrawLightButtonBackground(dc, rect, Themed, Focused, Selected, m_Hover);

	// Content
	CRect rectText(rect);
	rectText.DeflateRect(MARGIN+2, MARGIN);

	if (Selected)
		rectText.OffsetRect(1, 1);

	// Text
	COLORREF clrText = Themed ? Disabled ? 0xA4A2A0 : m_Hover ? 0xCC6633 : 0xCC3300 : GetSysColor(Disabled ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT);
	dc.SetTextColor(clrText);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);
	dc.DrawText(m_Value, rectText, DT_HIDEPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	dc.SelectObject(pOldFont);

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

	BitBlt(lpDrawItemStruct->hDC, 0, 0, rect.Width(), rect.Height(), dc.m_hDC, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
	DeleteDC(dc.Detach());
	DeleteObject(MemBitmap.Detach());
}

void CHeaderButton::SetValue(LPCWSTR Value, BOOL Repaint)
{
	m_Value = Value;
	m_ShowDropdown = TRUE;

	if (Repaint)
		GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CHeaderButton::GetPreferredSize(LPSIZE lpSize, INT& CaptionWidth)
{
	ASSERT(lpSize);

	*lpSize = LFGetApp()->m_DefaultFont.GetTextExtent(m_Value.IsEmpty() ? _T("Wy") : m_Value);

	lpSize->cx += m_ShowDropdown ? 3*MARGIN+14 : 2*MARGIN+5;
	lpSize->cy += 2*MARGIN;

	if (m_ShowDropdown)
	{
		CString Caption(m_Caption);
		if (!Caption.IsEmpty())
			Caption += _T(":");

		m_CaptionWidth = CaptionWidth = LFGetApp()->m_DefaultFont.GetTextExtent(Caption).cx;
	}
	else
	{
		m_CaptionWidth = CaptionWidth = 0;
	}
}

void CHeaderButton::GetCaption(CString& Caption, INT& CaptionWidth) const
{
	Caption = m_Caption;
	CaptionWidth = m_CaptionWidth;

	if (!Caption.IsEmpty())
		Caption += _T(":");
}


BEGIN_MESSAGE_MAP(CHeaderButton, CHoverButton)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(REQUEST_TOOLTIP_DATA, OnRequestTooltipData)
END_MESSAGE_MAP()

void CHeaderButton::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	if (m_ShowDropdown)
	{
		GetOwner()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
	else
	{
		CHoverButton::OnContextMenu(pWnd, pos);
	}
}

void CHeaderButton::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	wcscpy_s(pTooltipData->Caption, 256, m_Caption);
	wcscpy_s(pTooltipData->Hint, 4096, m_Hint);

	*pResult = TRUE;
}
