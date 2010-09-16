
// CTooltipHeader.cpp: Implementierung der Klasse CTooltipHeader
//

#include "stdafx.h"
#include "CTooltipHeader.h"
#include "LFApplication.h"


// CTooltipHeader
//

CTooltipHeader::CTooltipHeader()
	: CHeaderCtrl()
{
	m_Hover = FALSE;
	m_HoverItem = -1;
}

CTooltipHeader::~CTooltipHeader()
{
}

void CTooltipHeader::PreSubclassWindow()
{
	m_TooltipCtrl.Create(this);
}

BOOL CTooltipHeader::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
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

	return CHeaderCtrl::PreTranslateMessage(pMsg);
}


BEGIN_MESSAGE_MAP(CTooltipHeader, CHeaderCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

void CTooltipHeader::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
	else
		if (m_TooltipCtrl.IsWindowVisible())
		{
			HDHITTESTINFO htt;
			htt.pt = point;

			int Item = HitTest(&htt);
			if (Item!=m_HoverItem)
				m_TooltipCtrl.Deactivate();
		}

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CTooltipHeader::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;

	CHeaderCtrl::OnMouseLeave();
}

void CTooltipHeader::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		HDHITTESTINFO htt;
		htt.pt = point;

		m_HoverItem = HitTest(&htt);
		if (m_HoverItem!=-1)
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				HDITEMW i;
				i.mask = HDI_TEXT;
				i.pszText = m_TooltipTextBuffer;
				i.cchTextMax = 256;

				if (GetItem(m_HoverItem, &i))
					if (m_TooltipTextBuffer[0]!=L'\0')
					{
						ClientToScreen(&point);
						m_TooltipCtrl.Track(point, NULL, NULL, _T(""), m_TooltipTextBuffer);
					}
			}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}
