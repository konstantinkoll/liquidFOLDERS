
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskButton
//

#define BORDER     4

CTaskButton::CTaskButton()
	: CHoverButton()
{
}

BOOL CTaskButton::Create(CWnd* pParentWnd, UINT nID, CString Caption, CString TooltipHeader, CString TooltipHint, CMFCToolBarImages* Icons, INT IconSize, INT IconID)
{
	m_Caption = Caption;
	m_TooltipHeader = TooltipHeader;
	m_TooltipHint = TooltipHint;
	p_Icons = Icons;
	m_IconSize = IconSize;
	m_IconID = IconID;
	m_OverlayID = -1;

	CRect rect;
	rect.SetRectEmpty();
	return CHoverButton::Create(TooltipHeader, WS_VISIBLE | WS_DISABLED | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, rect, pParentWnd, nID);
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
		m_TooltipCtrl.Deactivate();
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

INT CTaskButton::GetPreferredWidth()
{
	INT Width = 2*(BORDER+2);

	if ((p_Icons) && (m_IconID!=-1))
		Width += m_IconSize+(m_Caption.IsEmpty() ? 0 : BORDER);

	if (!m_Caption.IsEmpty())
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
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

INT CTaskButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHoverButton::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

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
	if ((p_Icons) && (m_IconID!=-1))
	{
		CAfxDrawState ds;
		p_Icons->PrepareDrawImage(ds);

		CPoint pt(rectText.left, (rect.Height()-m_IconSize)/2+(Selected ? 2 : 1));
		p_Icons->Draw(&dc, pt.x, pt.y, m_IconID);

		if (m_OverlayID!=-1)
			p_Icons->Draw(&dc, pt.x+5, pt.y-(m_IconSize==32 ? 2 : 3), m_OverlayID);

		p_Icons->EndDrawImage(ds);

		rectText.left += m_IconSize+BORDER;
	}

	// Text
	dc.SetTextColor(Themed ? m_Hover ? 0x404040 : 0x333333 : GetSysColor(COLOR_WINDOWTEXT));

	HFONT hOldFont = (HFONT)dc.SelectObject(Themed ? LFGetApp()->m_DefaultFont.m_hObject : GetStockObject(DEFAULT_GUI_FONT));
	dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	dc.SelectObject(hOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CTaskButton::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();

	CHoverButton::OnMouseLeave();
}

void CTaskButton::OnMouseHover(UINT nFlags, CPoint point)
{
	if (!m_TooltipHeader.IsEmpty())
		if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
		{
			ClientToScreen(&point);
			m_TooltipCtrl.Track(point, NULL, m_TooltipHint.IsEmpty() ? _T("") : m_TooltipHeader, m_TooltipHint.IsEmpty() ? m_TooltipHeader : m_TooltipHint);
		}
		else
		{
			m_TooltipCtrl.Deactivate();
		}
}
