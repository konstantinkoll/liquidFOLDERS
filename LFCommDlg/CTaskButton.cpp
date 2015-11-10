
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskButton
//

#define BORDER     4

BOOL CTaskButton::Create(CWnd* pParentWnd, UINT nID, const CString& Caption, const CString& Hint, CIcons* pButtonIcons, CIcons* pTooltipIcons, INT IconSize, INT IconID, BOOL ForceSmall, BOOL HideIcon)
{
	m_Caption = Caption;
	m_Hint = Hint;
	p_ButtonIcons = pButtonIcons;
	p_TooltipIcons = pTooltipIcons;
	m_IconSize = IconSize;
	m_IconID = IconID;
	m_ForceSmall = ForceSmall;
	m_HideIcon = HideIcon;

	return CHoverButton::Create(Caption, WS_VISIBLE | WS_DISABLED | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CTaskButton::PreTranslateMessage(MSG* pMsg)
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
		LFGetApp()->HideTooltip();
		break;
	}

	return CHoverButton::PreTranslateMessage(pMsg);
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
		Width += m_IconSize;

		if (!m_Small)
			Width += BORDER;
	}

	if (!m_Small)
		Width += LFGetApp()->m_DefaultFont.GetTextExtent(m_Caption).cx;

	return Width;
}


BEGIN_MESSAGE_MAP(CTaskButton, CHoverButton)
	ON_WM_PAINT()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

void CTaskButton::OnPaint()
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
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Button
	BOOL Themed = IsCtrlThemed();
	BOOL Selected = (GetState() & 4);

	DrawLightButtonBackground(dc, rect, Themed, GetState() & 8, Selected, m_Hover);

	// Content
	CRect rectText(rect);
	rectText.DeflateRect(BORDER+2, BORDER);
	if (Selected)
		rectText.OffsetRect(1, 1);

	// Icon
	if (p_ButtonIcons && (!m_HideIcon || m_Small))
	{
		p_ButtonIcons->Draw(dc, rectText.left, (rect.Height()-m_IconSize)/2+(Selected ? 2 : 1), m_IconID);
		rectText.left += m_IconSize+BORDER;
	}

	// Text
	if (!m_Small)
	{
		dc.SetTextColor(Themed ? m_Hover ? 0x404040 : 0x333333 : GetSysColor(COLOR_WINDOWTEXT));

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);
		dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX);
		dc.SelectObject(pOldFont);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CTaskButton::OnMouseLeave()
{
	LFGetApp()->HideTooltip();

	CHoverButton::OnMouseLeave();
}

void CTaskButton::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		LFGetApp()->ShowTooltip(this, point, m_Caption, m_Hint, p_TooltipIcons->ExtractIcon(m_IconID));
	}
	else
	{
		LFGetApp()->HideTooltip();
	}
}
