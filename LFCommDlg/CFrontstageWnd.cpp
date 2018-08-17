
// CFrontstageWnd.cpp: Implementierung der Klasse CFrontstageWnd
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageWnd
//

#define BORDERSIZE            2
#define WHITEAREAHEIGHT     100

CFrontstageWnd::CFrontstageWnd()
	: CWnd()
{
	CONSTRUCTOR_TOOLTIP()
}


// Cards

void CFrontstageWnd::DrawCardBackground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed) const
{
	ASSERT(lpRect);

	dc.FillSolidRect(lpRect, Themed ? 0xF8F5F4 : GetSysColor(COLOR_3DFACE));

	if (Themed)
	{
		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.SetSmoothingMode(SmoothingModeNone);

		LinearGradientBrush brush(Point(0, lpRect->top), Point(0, lpRect->top+WHITEAREAHEIGHT), Color(0xFFFFFFFF), Color(0xFFF4F5F8));
		g.FillRectangle(&brush, Rect(lpRect->left, lpRect->top, lpRect->right-lpRect->left, WHITEAREAHEIGHT));

		g.SetSmoothingMode(SmoothingModeAntiAlias);
	}
}

void CFrontstageWnd::DrawCardForeground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed, BOOL Hot, BOOL Focused, BOOL Selected, COLORREF TextColor, BOOL ShowFocusRect) const
{
	// Shadow
	GraphicsPath Path;

	if (Themed)
	{
		CRect rectShadow(lpRect);
		rectShadow.OffsetRect(1, 1);

		CreateRoundRectangle(rectShadow, 3, Path);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

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
			dc.Draw3dRect(lpRect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
		}
	}

	DrawListItemBackground(dc, lpRect, Themed, GetFocus()==this, Hot, Focused, Selected, TextColor, ShowFocusRect);
}

void CFrontstageWnd::DrawWindowEdge(Graphics& g, BOOL Themed) const
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

			g.SetPixelOffsetMode(PixelOffsetModeNone);
			g.SetSmoothingMode(LFGetApp()->m_SmoothingModeAntiAlias8x8);

			GraphicsPath path;
			CreateRoundRectangle(rectOutline, BACKSTAGERADIUS, path);

			Pen pen(Color(0xFF000000));
			g.DrawPath(&pen, &path);
		}
	}
}

void CFrontstageWnd::DrawWindowEdge(CDC& dc, BOOL Themed) const
{
	if (Themed)
	{
		Graphics g(dc);

		DrawWindowEdge(g, Themed);
	}
}


// Items

INT CFrontstageWnd::ItemAtPosition(CPoint /*point*/) const
{
	return -1;
}

CPoint CFrontstageWnd::PointAtPosition(CPoint /*point*/) const
{
	return CPoint(-1, -1);
}

LPCVOID CFrontstageWnd::PtrAtPosition(CPoint /*point*/) const
{
	return NULL;
}

void CFrontstageWnd::InvalidateItem(INT /*Index*/)
{
	Invalidate();
}

void CFrontstageWnd::InvalidatePoint(const CPoint& /*Point*/)
{
	Invalidate();
}

void CFrontstageWnd::InvalidatePtr(LPCVOID /*Ptr*/)
{
	Invalidate();
}

void CFrontstageWnd::ShowTooltip(const CPoint& /*point*/)
{
}

void CFrontstageWnd::UpdateHoverItem()
{
	if (!GetCapture())
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		OnMouseMove(0, point);
	}
}


// Menus

BOOL CFrontstageWnd::GetContextMenu(CMenu& /*Menu*/, INT /*Index*/)
{
	return FALSE;
}

BOOL CFrontstageWnd::GetContextMenu(CMenu& /*Menu*/, LPCVOID /*Ptr*/)
{
	return FALSE;
}

BOOL CFrontstageWnd::GetContextMenu(CMenu& /*Menu*/, const CPoint& /*point*/)
{
	return FALSE;
}

void CFrontstageWnd::TrackPopupMenu(CMenu& Menu, const CPoint& pos, CWnd* pWndOwner, BOOL SetDefaultItem, BOOL AlignRight) const
{
	if (IsMenu(Menu))
	{
		if (SetDefaultItem)
			Menu.SetDefaultItem(0, TRUE);

		CMenu MenuPopup;
		if (MenuPopup.CreatePopupMenu())
		{
			MenuPopup.InsertMenu(0, MF_POPUP, (UINT_PTR)(HMENU)Menu);

			Menu.TrackPopupMenu((AlignRight ? TPM_RIGHTALIGN : TPM_LEFTALIGN) | TPM_RIGHTBUTTON, pos.x, pos.y, pWndOwner, NULL);
		}
	}
}


IMPLEMENT_TOOLTIP_NOWHEEL(CFrontstageWnd, CWnd)

BEGIN_TOOLTIP_MAP(CFrontstageWnd, CWnd)
	ON_WM_DESTROY()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_MESSAGE(WM_NCPAINT, OnNcPaint)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
END_TOOLTIP_MAP()

void CFrontstageWnd::OnDestroy()
{
	HideTooltip();

	CWnd::OnDestroy();
}

void CFrontstageWnd::OnNcCalcSize(BOOL bCalcValidRect, NCCALCSIZE_PARAMS* lpncsp)
{
	if (HasBorder())
	{
		const INT cxEdge = GetSystemMetrics(SM_CXEDGE);
		const INT cyEdge = GetSystemMetrics(SM_CYEDGE);

		lpncsp->rgrc[0].left += cxEdge;
		lpncsp->rgrc[0].right -= cxEdge;
		lpncsp->rgrc[0].top += cyEdge;
		lpncsp->rgrc[0].bottom -= cyEdge;
	}
	else
	{
		CWnd::OnNcCalcSize(bCalcValidRect, lpncsp);
	}
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

LRESULT CFrontstageWnd::OnNcPaint(WPARAM wParam, LPARAM lParam)
{
	if (HasBorder() && IsCtrlThemed())
	{
		// Window DC
		CWindowDC dc(this);

		// Window Rect
		const INT cxEdge = GetSystemMetrics(SM_CXEDGE);
		const INT cyEdge = GetSystemMetrics(SM_CYEDGE);

		CRect rect;
		GetWindowRect(rect);

		rect.DeflateRect(cxEdge, cyEdge);

		// Update region
		HRGN hRgn = CreateRectRgnIndirect(rect);

		HRGN hRgnUpdate = (HRGN)wParam;
		if (hRgnUpdate)
			CombineRgn(hRgn, hRgnUpdate, hRgn, RGN_AND);

		// Draw themed border
		rect.OffsetRect(-rect.left+cxEdge, -rect.top+cyEdge);
		dc.ExcludeClipRect(rect);

		rect.InflateRect(cxEdge, cyEdge);
		dc.Draw3dRect(rect, 0xA0A0A0, 0xA0A0A0);

		rect.DeflateRect(1, 1);
		dc.FillSolidRect(rect, IsWindowEnabled() ? 0xFFFFFF : GetSysColor(COLOR_3DFACE));

		// Draw rest of non-client area
		DefWindowProc(WM_NCPAINT, (WPARAM)hRgn, lParam);
		DeleteObject(hRgn);
	}
	else
	{
		DefWindowProc(WM_NCPAINT, wParam, lParam);
	}

	return 0;
}

BOOL CFrontstageWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

HBRUSH CFrontstageWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetDCBrushColor(IsWindowEnabled() ? IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOW) : GetSysColor(COLOR_3DFACE));

	return (HBRUSH)GetStockObject(DC_BRUSH);
}

void CFrontstageWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	INT Index = -1;
	LPCVOID Ptr = NULL;
	CPoint Point(-1, -1);

	// Handle click position
	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
	}
	else
	{
		ScreenToClient(&pos);

		Index = ItemAtPosition(pos);
		Ptr = PtrAtPosition(pos);
		Point = PointAtPosition(pos);
	}

	// Get menu
	CMenu Menu;
	BOOL SetDefaultItem = FALSE;

	SetDefaultItem |= GetContextMenu(Menu, Index);
	SetDefaultItem |= GetContextMenu(Menu, Ptr);
	SetDefaultItem |= GetContextMenu(Menu, Point);

	ClientToScreen(&pos);

	if (IsMenu(Menu))
	{
		TrackPopupMenu(Menu, pos, SetDefaultItem);
	}
	else
	{
		GetOwner()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pos.x, pos.y));
	}
}

void CFrontstageWnd::OnInitMenuPopup(CMenu* pMenuPopup, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pMenuPopup);

	CCmdUI cmdUI;
	cmdUI.m_pMenu = cmdUI.m_pParentMenu = pMenuPopup;
	cmdUI.m_nIndexMax = pMenuPopup->GetMenuItemCount();

	ASSERT(!cmdUI.m_pOther);
	ASSERT(cmdUI.m_pMenu);

	for (cmdUI.m_nIndex=0; cmdUI.m_nIndex<cmdUI.m_nIndexMax; cmdUI.m_nIndex++)
	{
		cmdUI.m_nID = pMenuPopup->GetMenuItemID(cmdUI.m_nIndex);

		if (cmdUI.m_nID && (cmdUI.m_nID!=(UINT)-1))
			cmdUI.DoUpdate(this, FALSE);
	}
}
