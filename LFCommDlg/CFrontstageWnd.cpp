
// CFrontstageWnd.cpp: Implementierung der Klasse CFrontstageWnd
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageWnd
//

#define WHITE    100

void CFrontstageWnd::DrawCardBackground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed)
{
	dc.FillSolidRect(lpRect, Themed ? 0xF8F5F4 : GetSysColor(COLOR_3DFACE));

	if (Themed)
	{
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		LinearGradientBrush brush(Point(0, 0), Point(0, WHITE), Color(0xFFFFFFFF), Color(0xFFF4F5F8));
		g.FillRectangle(&brush, Rect(0, 0, lpRect->right-lpRect->left, WHITE));

		g.SetPixelOffsetMode(PixelOffsetModeNone);
	}
}

void CFrontstageWnd::DrawCardForeground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed, BOOL Hot, BOOL Focused, BOOL Selected, COLORREF TextColor, BOOL ShowFocusRect)
{
	// Shadow
	GraphicsPath Path;

	if (Themed)
	{
		CRect rectShadow(lpRect);
		rectShadow.OffsetRect(1, 1);

		CreateRoundRectangle(rectShadow, 3, Path);

		g.SetSmoothingMode(SmoothingModeAntiAlias);

		Pen pen(Color(0x0C000000));
		g.DrawPath(&pen, &Path);
	}

	// Background
	if ((!Themed || !Hot) && !Selected)
	{
		CRect rect(lpRect);
		rect.DeflateRect(1, 1);

		dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

		if (Themed)
		{
			Matrix m;
			m.Translate(-1.0, -1.0);
			Path.Transform(&m);

			Pen pen(Color(0xFFD0D1D5));
			g.DrawPath(&pen, &Path);
		}
		else
		{
			dc.Draw3dRect(rect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
		}
	}

	DrawListItemBackground(dc, lpRect, Themed, GetFocus()==this, Hot, Focused, Selected, TextColor, ShowFocusRect);
}

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
