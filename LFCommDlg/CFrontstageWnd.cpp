
// CFrontstageWnd.cpp: Implementierung der Klasse CFrontstageWnd
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageWnd
//

CFrontstageWnd::CFrontstageWnd()
	: CWnd()
{
	CONSTRUCTOR_TOOLTIP()
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


// Item handling

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

void CFrontstageWnd::InvalidatePoint(const CPoint& /*point*/)
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

BOOL CFrontstageWnd::HasBorder(HWND hWnd)
{
	ASSERT(hWnd);

	return ((GetWindowLong(hWnd, GWL_STYLE) & (WS_CHILD | WS_BORDER))==(WS_CHILD | WS_BORDER)) || (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_CLIENTEDGE);
}


// Draw support

void CFrontstageWnd::DrawGlasBackground(CDC& dc, Graphics& g, LPCRECT lpcRect, BOOL Themed) const
{
	ASSERT(lpcRect);

	CRect rectBackground(lpcRect);
	rectBackground.DeflateRect(1, 1);

	if (Themed)
	{
		// Shadow
		CRect rectShadow(lpcRect);
		rectShadow.OffsetRect(1, 1);

		GraphicsPath pathOuter;
		CreateRoundRectangle(rectShadow, 7, pathOuter);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		Pen pen1(Color(0x0A000000));
		g.DrawPath(&pen1, &pathOuter);

		// Background
		CRect rect(lpcRect);
		rect.DeflateRect(1, 1);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.SetSmoothingMode(SmoothingModeNone);

		SolidBrush brush(Color(0xE4F8F9FC));
		g.FillRectangle(&brush, rectBackground.left+1, rectBackground.top+1, rectBackground.Width()-2, rectBackground.Height()-2);

		g.SetPixelOffsetMode(PixelOffsetModeNone);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		// Inner border
		GraphicsPath pathInner;
		CreateRoundRectangle(rectBackground, 6, pathInner);

		Pen pen3(Color(0xFFFFFFFF));
		g.DrawPath(&pen3, &pathInner);

		// Outer border
		Matrix m;
		m.Translate(-1.0f, -1.0f);
		pathOuter.Transform(&m);

		Pen pen2(Color(0xFFD0D1D5));
		g.DrawPath(&pen2, &pathOuter);
	}
	else
	{
		dc.FillSolidRect(rectBackground, GetSysColor(COLOR_WINDOW));
		dc.Draw3dRect(lpcRect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
	}
}

void CFrontstageWnd::DrawCardBackground(CDC& dc, Graphics& g, LPCRECT lpcRect, BOOL Themed) const
{
	ASSERT(lpcRect);

	if (Themed)
	{
		const INT Height = lpcRect->bottom-lpcRect->top;
		const INT Width = lpcRect->right-lpcRect->left;

		Bitmap* pImage = LFGetApp()->GetCachedResourceImage(IDB_BACKGROUND_CARDS);
		ASSERT(pImage);

		// Gradient
		const INT GradientHeight = pImage->GetHeight();

		ImageAttributes ImgAttr;
		ImgAttr.SetWrapMode(WrapModeTile);

		g.DrawImage(pImage, Rect(lpcRect->left, lpcRect->top, Width, GradientHeight), 0, 0, Width, GradientHeight, UnitPixel, &ImgAttr);

		// Bottom
		dc.FillSolidRect(lpcRect->left, lpcRect->top+GradientHeight, Width, Height-GradientHeight, 0xF8F5F4);
	}
	else
	{
		dc.FillSolidRect(lpcRect, GetSysColor(COLOR_3DFACE));
	}
}

void CFrontstageWnd::DrawCardForeground(CDC& dc, Graphics& g, LPCRECT lpcRect, BOOL Themed, BOOL Hot, BOOL Focused, BOOL Selected, COLORREF TextColor, BOOL ShowFocusRect) const
{
	// Shadow
	GraphicsPath path;

	if (Themed)
	{
		CRect rectShadow(lpcRect);
		rectShadow.OffsetRect(1, 1);

		CreateRoundRectangle(rectShadow, 3, path);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		Pen pen(Color(0x0C000000));
		g.DrawPath(&pen, &path);
	}

	// Background
	if ((!Themed || !Hot) && !Selected)
	{
		CRect rect(lpcRect);
		rect.DeflateRect(1, 1);

		dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

		if (Themed)
		{
			Matrix m;
			m.Translate(-1.0f, -1.0f);
			path.Transform(&m);

			Pen pen(Color(0xFFD0D1D5));
			g.DrawPath(&pen, &path);
		}
		else
		{
			dc.Draw3dRect(lpcRect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
		}
	}

	DrawListItemBackground(dc, lpcRect, Themed, GetFocus()==this, Hot, Focused, Selected, TextColor, ShowFocusRect);
}

void CFrontstageWnd::DrawWindowEdge(Graphics& g) const
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

void CFrontstageWnd::DrawWindowEdge(CDC& dc, BOOL Themed) const
{
	if (Themed)
	{
		Graphics g(dc);

		DrawWindowEdge(g);
	}
}


IMPLEMENT_TOOLTIP_NOWHEEL(CFrontstageWnd, CWnd)

BEGIN_TOOLTIP_MAP(CFrontstageWnd, CWnd)
	ON_WM_DESTROY()
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
	if (HasBorder())
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
