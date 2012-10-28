
// CTooltipList.cpp: Implementierung der Klasse CTooltipList
//

#include "stdafx.h"
#include "CTooltipList.h"


// CTooltipList
//

CTooltipList::CTooltipList()
	: CExplorerList()
{
	m_Hover = FALSE;
	m_HoverItem = m_TooltipItem = -1;
}

BOOL CTooltipList::PreTranslateMessage(MSG* pMsg)
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
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CExplorerList::PreTranslateMessage(pMsg);
}

void CTooltipList::Init()
{
	CExplorerList::Init();

	// Tooltip
	m_TooltipCtrl.Create(this);
}


BEGIN_MESSAGE_MAP(CTooltipList, CExplorerList)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

void CTooltipList::OnMouseMove(UINT nFlags, CPoint point)
{
	LVHITTESTINFO htt;
	htt.pt = point;
	m_HoverItem = HitTest(&htt);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (m_HoverItem!=m_TooltipItem))
			m_TooltipCtrl.Deactivate();

	CExplorerList::OnMouseMove(nFlags, point);
}

void CTooltipList::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	m_HoverItem = -1;

	CExplorerList::OnMouseLeave();
}

void CTooltipList::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		LVHITTESTINFO htt;
		htt.pt = point;

		m_TooltipItem = HitTest(&htt);
		if (m_TooltipItem!=-1)
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				NM_TOOLTIPDATA tag;
				ZeroMemory(&tag, sizeof(tag));
				tag.hdr.code = REQUEST_TOOLTIP_DATA;
				tag.hdr.hwndFrom = m_hWnd;
				tag.hdr.idFrom = GetDlgCtrlID();
				tag.Item = m_TooltipItem;

				GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
				if (tag.Show)
				{
					ClientToScreen(&point);
					m_TooltipCtrl.Track(point, tag.hIcon, CSize(tag.cx, tag.cy), GetItemText(m_TooltipItem, 0), tag.Text);
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
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}
