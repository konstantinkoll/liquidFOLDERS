
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
	Tracking = -1;
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
		Tracking = 1;
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
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);

		MouseInWnd = TRUE;
	}

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CTooltipHeader::OnMouseLeave()
{
	Tooltip.Deactivate();
	Tracking = -1;

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
			if (rect.PtInRect(point) && (a!=Tracking))
			{
				HDITEMW i;
				i.mask = HDI_TEXT;
				i.pszText = TooltipText;
				i.cchTextMax = 256;

				if (GetItem(a, &i))
				{
					ClientToScreen(rect);
					rect.OffsetRect(1, rect.Height());

					Tooltip.Hide();
					Tooltip.Track(rect, TooltipText);

					Tracking = a;
				}

				break;
			}
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}
