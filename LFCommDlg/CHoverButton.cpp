
// CHoverButton.cpp: Implementierung der Klasse CHoverButton
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CHoverButton
//

CHoverButton::CHoverButton()
	: CButton()
{
	m_Hover = FALSE;
}

BOOL CHoverButton::Create(LPCTSTR lpszCaption, CWnd* pParentWnd, UINT nID)
{
	return CButton::Create(lpszCaption, WS_VISIBLE | WS_TABSTOP | WS_GROUP | BS_OWNERDRAW, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CHoverButton::PreTranslateMessage(MSG* pMsg)
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

	return CButton::PreTranslateMessage(pMsg);
}


BEGIN_MESSAGE_MAP(CHoverButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CHoverButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHoverButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CButton::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CHoverButton::OnMouseLeave()
{
	LFGetApp()->HideTooltip();
	m_Hover = FALSE;

	Invalidate();
}

void CHoverButton::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (!LFGetApp()->IsTooltipVisible())
		{
			NM_TOOLTIPDATA tag;
			ZeroMemory(&tag, sizeof(tag));

			tag.hdr.code = REQUEST_TOOLTIP_DATA;
			tag.hdr.hwndFrom = m_hWnd;
			tag.hdr.idFrom = GetDlgCtrlID();

			if (GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag)))
				LFGetApp()->ShowTooltip(this, point, tag.Caption, tag.Hint, tag.hIcon, tag.hBitmap);
		}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}
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
