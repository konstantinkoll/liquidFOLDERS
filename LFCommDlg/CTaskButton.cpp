
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskButton
//

#define BORDER     4

BOOL CTaskButton::Create(CWnd* pParentWnd, UINT nID, CString Caption, CString Hint, CMFCToolBarImages* pButtonIcons, CMFCToolBarImages* pTooltipIcons, INT IconSize, INT IconID, BOOL ForceSmall, BOOL HideIcon)
{
	m_Caption = Caption;
	m_Hint = Hint;
	p_ButtonIcons = pButtonIcons;
	p_TooltipIcons = pTooltipIcons;
	m_IconSize = IconSize;
	m_IconID = IconID;
	m_ForceSmall = ForceSmall;
	m_HideIcon = HideIcon;
	m_OverlayID = -1;

	CRect rect;
	rect.SetRectEmpty();
	return CHoverButton::Create(Caption, WS_VISIBLE | WS_DISABLED | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, rect, pParentWnd, nID);
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

void CTaskButton::SetIconID(INT IconID, INT OverlayID)
{
	m_IconID = IconID;
	m_OverlayID = OverlayID;

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
	{
		CDC* pDC = GetDC();
		HFONT hOldFont = (HFONT)pDC->SelectObject(IsCtrlThemed() ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
		Width += pDC->GetTextExtent(m_Caption).cx;
		pDC->SelectObject(hOldFont);
		ReleaseDC(pDC);
	}

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
	HBRUSH hBrush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (hBrush)
		FillRect(dc, rect, hBrush);

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
		CAfxDrawState ds;
		p_ButtonIcons->PrepareDrawImage(ds);

		CPoint pt(rectText.left, (rect.Height()-m_IconSize)/2+(Selected ? 2 : 1));
		p_ButtonIcons->Draw(&dc, pt.x, pt.y, m_IconID);

		if (m_OverlayID!=-1)
			p_ButtonIcons->Draw(&dc, pt.x+5, pt.y-(m_IconSize==32 ? 2 : 3), m_OverlayID);

		p_ButtonIcons->EndDrawImage(ds);

		rectText.left += m_IconSize+BORDER;
	}

	// Text
	if (!m_Small)
	{
		dc.SetTextColor(Themed ? m_Hover ? 0x404040 : 0x333333 : GetSysColor(COLOR_WINDOWTEXT));

		HFONT hOldFont = (HFONT)dc.SelectObject(Themed ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
		dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
		dc.SelectObject(hOldFont);
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
