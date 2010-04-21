
// CGlassButton.cpp: Implementierung der Klasse CGlassButton
//

#include "stdafx.h"
#include "CGlassButton.h"
#include "LFDialog.h"


// CGlassButton
//

CGlassButton::CGlassButton()
	: CButton()
{
	m_Hover = FALSE;
}

CGlassButton::~CGlassButton()
{
}

void CGlassButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// Background
	CRect rectWindow;
	GetWindowRect(rectWindow);
	GetParent()->ScreenToClient(rectWindow);

	CDC dcBack;
	dcBack.CreateCompatibleDC(pDC);
	CBitmap* oldBitmap = (CBitmap*)dcBack.SelectObject(((LFDialog*)GetParent())->GetBackBuffer());
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcBack, rectWindow.left, rectWindow.top, SRCCOPY);
	dcBack.SelectObject(oldBitmap);

	Graphics g(dc.m_hDC);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	// Outer border
	CRect rectBounds = rect;
	rectBounds.right--;
	rectBounds.bottom--;

	GraphicsPath path;
	CreateRoundRectangle(rectBounds, 4, path);
	if (lpDIS->itemState & ODS_FOCUS)
	{
		Pen pen(Color(0x3C, 0x7F, 0xB1));
		g.DrawPath(&pen, &path);
	}
	else
	{
		Pen pen(Color(0x70, 0x70, 0x70));
		g.DrawPath(&pen, &path);
	}

	// Content
	rectBounds.DeflateRect(1, 1);

	CreateRoundRectangle(rectBounds, 2, path);
	if (m_Hover)
	{
		SolidBrush sBr(Color((lpDIS->itemState & ODS_SELECTED) ? 0x60 : 0x40, 0x00, 0x93, 0xE7));
		g.FillPath(&sBr, &path);
	}
	else
	{
		SolidBrush sBr(Color(0x2A, 0x09, 0x09, 0x09));
		g.FillPath(&sBr, &path);
	}

	// Glow
	if (m_Hover)
	{
		g.SetClip(&path);
		GraphicsPath brad;
		CreateBottomRadialPath(rectBounds, brad);
		PathGradientBrush pgBr(&brad);
		RectF bounds;
		brad.GetBounds(&bounds);
		pgBr.SetCenterPoint(PointF((bounds.GetLeft()+bounds.GetRight())/2.0f, (bounds.GetTop()+bounds.GetBottom())/2.0f));
		pgBr.SetCenterColor(Color(0x80, 0x40, 0x80, 0xFF));
		INT cCols = 1;
		const Color cols[] = { Color(0x00, 0x40, 0x80, 0xFF) };
		pgBr.SetSurroundColors(cols, &cCols);
		g.FillPath(&pgBr, &brad);
		g.ResetClip();
	}

	// Shine
	CRect rectShine = rectBounds;
	rectShine.bottom -= rectShine.Height()/2-1;

	BYTE opacity = (lpDIS->itemState & ODS_SELECTED) ? 0x80 : 0x99;
	CreateRoundRectangle(rectShine, 2, path);
	LinearGradientBrush lgBr(Rect(rectShine.left, rectShine.top, rectShine.Width(), rectShine.Height()),
		Color(opacity, 0xFF, 0xFF, 0xFF),
		Color(opacity/3, 0xFF, 0xFF, 0xFF),
		LinearGradientModeVertical);
	g.FillPath(&lgBr, &path);

	// Text
	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
	CString tmpStr;
	GetWindowText(tmpStr);
	rectBounds.top++;
	dc.SetTextColor(0x000000);
	dc.DrawText(tmpStr, rectBounds, DT_SINGLELINE | DT_CENTER | DT_END_ELLIPSIS | DT_VCENTER);
	dc.SelectObject(pOldFont);
	rectBounds.top--;

	// Inner border
	CreateRoundRectangle(rectBounds, 3, path);
	if (((lpDIS->itemState & ODS_FOCUS) && (!m_Hover)) || (lpDIS->itemState & ODS_SELECTED))
	{
		Pen pen(Color(0x2D, 0xD4, 0xFF));
		g.DrawPath(&pen, &path);
	}
	else
		if (m_Hover)
		{
			Pen pen(Color(0xC0, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
		}
		else
		{
			Pen pen(Color(0xFF, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
		}

	// Focus rect
	if (lpDIS->itemState & ODS_FOCUS)
	{
		rectBounds.top++;
		rectBounds.left++;
		dc.DrawFocusRect(rectBounds);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}


BEGIN_MESSAGE_MAP(CGlassButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

BOOL CGlassButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGlassButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CButton::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
}

LRESULT CGlassButton::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_Hover = FALSE;
	Invalidate();

	return NULL;
}

void CGlassButton::CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path)
{
	path.Reset();

	int l = rect.left;
	int t = rect.top;
	int w = rect.Width();
	int h = rect.Height();
	int d = rad<<1;

	path.AddArc(l, t, d, d, 180, 90);
	path.AddLine(l+rad, t, l+w-rad, t);
	path.AddArc(l+w-d, t, d, d, 270, 90);
	path.AddLine(l+w, t+rad, l+w, t+h-rad);
	path.AddArc(l+w-d, t+h-d, d, d, 0, 90);
	path.AddLine(l+w-rad, t+h, l+rad, t+h);
	path.AddArc(l, t+h-d, d, d, 90, 90);
	path.AddLine(l, t+h-rad, l, t+rad);
	path.CloseFigure();
}

void CGlassButton::CreateBottomRadialPath(CRect rect, GraphicsPath& path)
{
	path.Reset();

	RectF rectF((REAL)rect.left, (REAL)rect.top, (REAL)rect.Width(), (REAL)rect.Height());
	//rectF.X -= rect.Width()*.35f;
	rectF.Y -= rect.Height()*.15f;
	//rectF.Width *= 1.7f;
	rectF.Height *= 2.3f;

	path.AddEllipse(rectF);
	path.CloseFigure();
}
