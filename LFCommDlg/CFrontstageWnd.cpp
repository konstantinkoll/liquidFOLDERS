
// CFrontstageWnd.cpp: Implementierung der Klasse CFrontstageWnd
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageWnd
//

void CFrontstageWnd::DrawWindowEdge(Graphics& g, BOOL Themed)
{
	if (Themed)
	{
		CWnd* pRootWnd = GetAncestor(GA_ROOT);
		ASSERT(pRootWnd);

		if (!pRootWnd->IsZoomed())
		{
			CRect rectOutline;
			pRootWnd->GetClientRect(rectOutline);
			pRootWnd->ClientToScreen(rectOutline);
			ScreenToClient(rectOutline);

			rectOutline.InflateRect(1, 1);

			g.SetSmoothingMode(LFGetApp()->m_SmoothingModeAntiAlias8x8);

			GraphicsPath path;
			CreateRoundRectangle(rectOutline, BACKSTAGERADIUS, path);

			Pen pen(Color(0xFF000000));
			g.DrawPath(&pen, &path);
		}
	}
}

void CFrontstageWnd::DrawWindowEdge(CDC& dc, BOOL Themed)
{
	if (Themed)
	{
		Graphics g(dc);

		DrawWindowEdge(g, Themed);
	}
}


BEGIN_MESSAGE_MAP(CFrontstageWnd, CWnd)
	ON_MESSAGE(WM_NCCALCSIZE, OnNcCalcSize)
	ON_WM_NCHITTEST()
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

LRESULT CFrontstageWnd::OnNcCalcSize(WPARAM wParam, LPARAM lParam)
{
	CWnd::OnNcCalcSize((BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);

	if (wParam)
	{
		NCCALCSIZE_PARAMS* lpncsp = (NCCALCSIZE_PARAMS*)lParam;
		ZeroMemory(&lpncsp->rgrc[1], 2*sizeof(RECT));

		return WVR_VALIDRECTS;
	}

	return 0;
}

LRESULT CFrontstageWnd::OnNcHitTest(CPoint point)
{
	CRect rectWindow;
	GetAncestor(GA_ROOT)->GetWindowRect(rectWindow);

	if (!rectWindow.PtInRect(point))
		return HTNOWHERE;

	if ((point.x<rectWindow.left+BACKSTAGEGRIPPER) || (point.x>=rectWindow.right-BACKSTAGEGRIPPER) ||
		(point.y<rectWindow.top+BACKSTAGEGRIPPER) || (point.y>=rectWindow.bottom-BACKSTAGEGRIPPER))
		return HTTRANSPARENT;

	return CWnd::OnNcHitTest(point);
}

void CFrontstageWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI state;
	state.m_pMenu = state.m_pParentMenu = pPopupMenu;
	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();

	ASSERT(!state.m_pOther);
	ASSERT(state.m_pMenu);

	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if ((state.m_nID) && (state.m_nID!=(UINT)-1))
			state.DoUpdate(this, FALSE);
	}
}
