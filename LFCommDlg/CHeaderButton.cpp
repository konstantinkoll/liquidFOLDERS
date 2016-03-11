
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
	m_Hover = FALSE;
	m_Value = _T("?");
	m_ShowDropdown = TRUE;
}

BOOL CHeaderButton::Create(CWnd* pParentWnd, UINT nID, const CString& Caption, const CString& Hint)
{
	m_Caption = Caption;
	m_Hint = Hint;

	return CHoverButton::Create(Caption, pParentWnd, nID);
}

void CHeaderButton::SetValue(LPCWSTR Value, BOOL ShowDropdown, BOOL Repaint)
{
	m_Value = Value;
	m_ShowDropdown = ShowDropdown;

	if (Repaint)
		GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CHeaderButton::GetPreferredSize(LPSIZE lpSize, INT& CaptionWidth)
{
	*lpSize = LFGetApp()->m_DefaultFont.GetTextExtent(m_Value.IsEmpty() ? _T("Wy") : m_Value);

	lpSize->cx += m_ShowDropdown ? 3*MARGIN+14 : 2*MARGIN+5;
	lpSize->cy += 2*MARGIN;

	CString Caption(m_Caption);
	if (!Caption.IsEmpty())
		Caption += _T(":");

	m_CaptionWidth = CaptionWidth = LFGetApp()->m_DefaultFont.GetTextExtent(Caption).cx;
}

void CHeaderButton::GetCaption(CString& Caption, INT& CaptionWidth) const
{
	Caption = m_Caption;
	CaptionWidth = m_CaptionWidth;

	if (!Caption.IsEmpty())
		Caption += _T(":");
}


BEGIN_MESSAGE_MAP(CHeaderButton, CHoverButton)
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(REQUEST_TOOLTIP_DATA, OnRequestTooltipData)
END_MESSAGE_MAP()

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
	const BOOL Focused = (GetState() & 8);
	const BOOL Selected = (GetState() & 4);

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Button
	BOOL Themed = IsCtrlThemed();

	DrawLightButtonBackground(dc, rect, Themed, Focused, Selected, m_Hover);

	// Content
	CRect rectText(rect);
	rectText.DeflateRect(MARGIN+2, MARGIN);

	if (Selected)
		rectText.OffsetRect(1, 1);

	// Text
	COLORREF clrText = Themed ? m_Hover ? 0xCC6633 : 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT);
	dc.SetTextColor(clrText);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);
	dc.DrawText(m_Value, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
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

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

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
