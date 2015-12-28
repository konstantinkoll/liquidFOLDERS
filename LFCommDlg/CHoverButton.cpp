
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

void CHoverButton::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
}


BEGIN_MESSAGE_MAP(CHoverButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE(WM_ISHOVER, OnIsHover)
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
	m_Hover = FALSE;
	Invalidate();
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

LRESULT CHoverButton::OnIsHover(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return m_Hover;
}
