
// CProgressBar.cpp: Implementierung der Klasse CProgressBar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CProgressBar
//

CProgressBar::CProgressBar()
	: CStatic()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CProgressBar";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CProgressBar", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CProgressBar::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyle(0, WS_CLIPSIBLINGS | WS_DISABLED);
}


BEGIN_MESSAGE_MAP(CProgressBar, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CProgressBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CProgressBar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Bar
	dc.FillSolidRect(2, 2, rect.Width()-4, rect.Height()-4, 0xF7F4F4);
	dc.FillSolidRect(2, 2, rect.Width()/2, rect.Height()-4, 0x00FF00);

	// Chrome
/*	if (Themed)
	{
		Graphics g(dc);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.SetSmoothingMode(SmoothingModeNone);

		DrawWhiteButtonBorder(g, rect);

		CRect rectBorder(rect);
		rectBorder.DeflateRect(2, 2);

		LinearGradientBrush brush1(Point(0, rectBorder.top), Point(0, (rectBorder.top+rectBorder.bottom)/8+1), Color(0x40000000), Color(0x00000000));
		g.FillRectangle(&brush1, rectBorder.left, rectBorder.top, rectBorder.Width(), rectBorder.Height()/8);

		LinearGradientBrush brush2(Point(0, rectBorder.bottom-3), Point(0, rectBorder.bottom), Color(0x00000000), Color(0x10000000));
		g.FillRectangle(&brush2, rectBorder.left, rectBorder.bottom-3, rectBorder.Width(), 3);

		g.SetPixelOffsetMode(PixelOffsetModeNone);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		rectBorder.InflateRect(1, 1);

		GraphicsPath pathOuter;
		CreateRoundRectangle(rectBorder, 2, pathOuter);

		Pen pen(Color(0xFFBCBDBE));
		g.DrawPath(&pen, &pathOuter);
	}
	else
	{
		dc.DrawEdge(rect, EDGE_SUNKEN, BF_RECT | BF_SOFT);
	}*/
	if (Themed)
	{
		Graphics g(dc);

		DrawWhiteButtonBorder(g, rect, FALSE);

		rect.top++;
		rect.left++;

		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		GraphicsPath PathInner;
		CreateRoundRectangle(rect, 2, PathInner);

		SolidBrush brush(Color(0x00, 0xC0, 0x00));
		g.FillPath(&brush, &PathInner);

//		if (Enabled)
		{
			// Boost reflection for light colors
			LinearGradientBrush brush1(Point(0, rect.top), Point(0, (rect.top+rect.bottom)/2+1), Color(0x30FFFFFF), Color(0x20FFFFFF));
			g.FillRectangle(&brush1, rect.left, rect.top, rect.Width()-1, rect.Height()/2);

			LinearGradientBrush brush2(Point(0, rect.bottom-6), Point(0, rect.bottom), Color(0x00000000), Color(0x20000000));
			g.FillRectangle(&brush2, rect.left+1, rect.bottom-6, rect.Width()-3, 5);
		}

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		rect.right--;
		rect.bottom--;

		GraphicsPath PathOuter;
		CreateRoundRectangle(rect, 2, PathOuter);

		Pen pen(Color(0x20000000));
		g.DrawPath(&pen, &PathOuter);
	}
	else
	{
		dc.FillSolidRect(rect, 0x00C000);

		dc.DrawEdge(rect, EDGE_SUNKEN, BF_RECT);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}
