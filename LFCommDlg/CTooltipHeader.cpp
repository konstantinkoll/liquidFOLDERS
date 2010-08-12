
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
	MouseInWnd = FALSE;
}

CTooltipHeader::~CTooltipHeader()
{
}

void CTooltipHeader::PreSubclassWindow()
{
	Tooltip.Create(this);
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
		Tooltip.Hide();
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
	if (!MouseInWnd)
	{
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);

		MouseInWnd = TRUE;
	}

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CTooltipHeader::OnMouseLeave()
{
	Tooltip.Deactivate();
	MouseInWnd = FALSE;

	CHeaderCtrl::OnMouseLeave();
}

void CTooltipHeader::OnMouseHover(UINT /*nFlags*/, CPoint point)
{
	int cnt = GetItemCount();
	for (int a=0; a<cnt; a++)
	{
		CRect rect;
		if (GetItemRect(a, rect))
			if (rect.PtInRect(point))
			{
				HDITEMW i;
				i.mask = HDI_TEXT;
				i.pszText = TooltipText;
				i.cchTextMax = 256;

				if (GetItem(a, &i))
				{
					ClientToScreen(&point);
					Tooltip.Track(point, TooltipText);
				}

				break;
			}
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}
