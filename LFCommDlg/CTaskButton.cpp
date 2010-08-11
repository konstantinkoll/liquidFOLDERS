
// CTaskButton.cpp: Implementierung der Klasse CTaskButton
//

#include "stdafx.h"
#include "CTaskbar.h"
#include "CTaskButton.h"
#include "LFApplication.h"


// CTaskButton
//

#define BORDER          4

CTaskButton::CTaskButton()
	: CButton()
{
	m_Hover = FALSE;
}

void CTaskButton::Create(CString Caption, CString Tooltip, CMFCToolBarImages* Icons, int IconID, CWnd* pParentWnd, UINT nID)
{
	m_Caption = Caption;
	m_Tooltip = Tooltip;
	m_Icons = Icons;
	m_IconID = IconID;

	CRect rect;
	rect.SetRectEmpty();
	CButton::Create(Caption, WS_VISIBLE | WS_TABSTOP | WS_GROUP, rect, pParentWnd, nID);
}

int CTaskButton::GetPreferredWidth()
{
	int l = 2*(BORDER+2)+1;

	if ((m_Icons) && (m_IconID!=-1))
		l += 16+(m_Caption.IsEmpty() ? 0 : BORDER);

	if (!m_Caption.IsEmpty())
	{
		CSize sz;

		CDC* dc = GetDC();
		CFont* pOldFont = dc->SelectObject(&((LFApplication*)AfxGetApp())->m_DefaultFont);
		dc->GetTextExtent(m_Caption);
		dc->SelectObject(pOldFont);
		ReleaseDC(dc);

		l += sz.cx;
	}

	return l;
}


BEGIN_MESSAGE_MAP(CTaskButton, CButton)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

BOOL CTaskButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTaskButton::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// State
	BOOL Focused = (GetState() & 8);
	BOOL Selected = (GetState() & 4);

	// Background
	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	Graphics g(dc.m_hDC);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_DefaultFont);

	OSVERSIONINFO i = ((LFApplication*)AfxGetApp())->osInfo;
	if ((i.dwMajorVersion>=6) && (i.dwMinorVersion!=0))
	{
	}
	else
	{
		if ((Focused) || (Selected) || (m_Hover))
		{
			// Outer border
			CRect rectBounds(rect);
			rectBounds.right--;
			rectBounds.bottom--;

			GraphicsPath path;
			CreateRoundRectangle(rectBounds, 4, path);
	
			Pen pen(Color(0x58, 0x00, 0x00, 0x00));
			g.DrawPath(&pen, &path);

			// Inner border
			rectBounds.DeflateRect(1, 1);
			CreateRoundRectangle(rectBounds, 2, path);

			if ((Focused) || (Selected))
			{
				// Shine
				Color c1;
				Color c2;
				if (Selected)
				{
					c1 = Color(0x20, 0x00, 0x00, 0x00);
					c2 = Color(0x40, 0x00, 0x00, 0x00);
				}
				else
				{
					c1 = Color(0x40, 0xFF, 0xFF, 0xFF);
					c2 = Color(0x00, 0xFF, 0xFF, 0xFF);
				}

				LinearGradientBrush brush(Point(0, rectBounds.top), Point(0, rectBounds.bottom), c1, c2);
				g.FillRectangle(&brush, rectBounds.top, rectBounds.left, rectBounds.Width(), rectBounds.Height());
			}

			pen.SetColor(Color(0x58, 0xFF, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
		}

		CRect rectText(rect);
		rectText.DeflateRect(BORDER+2, BORDER);
		if (Selected)
			rectText.OffsetRect(1, 1);

		if ((m_Icons) && (m_IconID!=-1))
		{
			CAfxDrawState ds;
			m_Icons->PrepareDrawImage(ds);
			m_Icons->Draw(&dc, rectText.left, (rect.Height()-rectText.Height())/2+(Selected ? 1 : 0), m_IconID);
			m_Icons->EndDrawImage(ds);

			rectText.left += 16+BORDER;
		}

		rectText.OffsetRect(1, 1);
		dc.SetTextColor(0x000000);
		dc.DrawText(m_Caption, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

		rectText.OffsetRect(-1, -1);
		dc.SetTextColor(0xFFFFFF);
		dc.DrawText(m_Caption, -1, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}


	/*
	if (m_Hover)
	{
		SolidBrush sBr(Color(Selected ? 0x60 : 0x40, 0x00, 0x93, 0xE7));
		g.FillPath(&sBr, &path);
	}
	else
	{
		SolidBrush sBr(Color(0x021, 0x30, 0x10, 0x00));
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

	BYTE opacity = Selected ? 0x80 : 0x99;
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
	if ((Focused && (!m_Hover)) || Selected || (GetStyle() & BS_DEFPUSHBUTTON))
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
			Pen pen(Color(0xF0, 0xF0, 0xF0));
			g.DrawPath(&pen, &path);
		}
*/
	// Focus rect

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
	dc.SelectObject(pOldFont);
}

void CTaskButton::OnMouseMove(UINT nFlags, CPoint point)
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

LRESULT CTaskButton::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_Hover = FALSE;
	Invalidate();

	return NULL;
}

void CTaskButton::OnEnable(BOOL bEnable)
{
	CButton::OnEnable(bEnable);
	GetParent()->SendMessage(WM_COMMAND, ID_UPDATEBUTTONS);
}

void CTaskButton::CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path)
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
