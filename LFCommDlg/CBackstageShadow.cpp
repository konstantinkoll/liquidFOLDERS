
// CBackstageShadow.cpp: Implementierung der Klasse CBackstageShadow
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageShadow
//

#define SHADOWSIZE       20
#define SHADOWOFFSET     SHADOWSIZE/2-2
#define SIDEWIDTH        (SHADOWSIZE+1)
#define TOPHEIGHT        (SIDEWIDTH-SHADOWOFFSET+BACKSTAGERADIUS)
#define BOTTOMHEIGHT     (SIDEWIDTH+BACKSTAGERADIUS)

CBackstageShadow::CBackstageShadow()
{
	m_Width = m_Height = 0;
	ZeroMemory(&m_wndTopLeft, sizeof(m_wndTopLeft));
}

BOOL CBackstageShadow::Create()
{
	CString className = AfxRegisterWndClass(0);

	BOOL Result = TRUE;

	for (UINT a=0; a<4; a++)
		m_wndShadow[a].CreateEx(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT, className, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL);

	return Result;
}

__forceinline void CBackstageShadow::Update(UINT nID, CDC& dc, POINT ptSrc, SIZE szWindow, CWnd* pBackstageWnd, const CRect& rectWindow)
{
	ASSERT(nID<4);

	m_wndTopLeft[nID] = ptSrc;

	CPoint ptDst(ptSrc.x+rectWindow.left, ptSrc.y+rectWindow.top);
	m_wndShadow[nID].UpdateLayeredWindow(&dc, &ptDst, &szWindow, &dc, &ptSrc, 0x000000, &BF, ULW_ALPHA);

	m_wndShadow[nID].SetWindowPos(pBackstageWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
}

void CBackstageShadow::Update(CWnd* pBackstageWnd, CRect rectWindow)
{
	ASSERT(IsWindow(m_hWnd));
	ASSERT(pBackstageWnd);

	rectWindow.InflateRect(SIDEWIDTH, SIDEWIDTH);
	rectWindow.top += SHADOWOFFSET;

	// Show window
	BOOL Visible = pBackstageWnd->IsWindowVisible() && !pBackstageWnd->IsZoomed() && !pBackstageWnd->IsIconic() && IsCtrlThemed();

	if (Visible && ((rectWindow.Width()!=m_Width) || (rectWindow.Height()!=m_Height)))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		// Prepare paint
		HBITMAP hWindowBitmap = CreateTransparentBitmap(rectWindow.Width(), rectWindow.Height());
		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hWindowBitmap);

		CRect rectBitmap(0, 0, rectWindow.Width(), rectWindow.Height());
		CRect rect(rectBitmap);

		Graphics g(dc);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;

		for (UINT a=0; a<SHADOWSIZE; a++)
		{
			CreateRoundRectangle(rect, BACKSTAGERADIUS+1+SHADOWSIZE-a, path);

			Pen pen(Color((BYTE)(((a+3)*(a+4)*(a+3)>>6)), 0x00, 0x00, 0x00));
			g.DrawPath(&pen, &path);

			rect.DeflateRect(1, 1);
		}

		rect.top -= SHADOWOFFSET;
		CreateRoundRectangle(rect, BACKSTAGERADIUS, path);

		Pen pen(Color(0x00, 0x00, 0x00));
		g.DrawPath(&pen, &path);

		// Update system-managed bitmap of window
		m_Width = rectWindow.Width();
		m_Height = rectWindow.Height();

		Update(0, dc, CPoint(0, 0), CSize(m_Width, TOPHEIGHT), pBackstageWnd, rectWindow);
		Update(1, dc, CPoint(0, TOPHEIGHT), CSize(SIDEWIDTH, m_Height-TOPHEIGHT-BOTTOMHEIGHT),pBackstageWnd, rectWindow);
		Update(2, dc, CPoint(m_Width-SIDEWIDTH, TOPHEIGHT), CSize(SIDEWIDTH, m_Height-TOPHEIGHT-BOTTOMHEIGHT), pBackstageWnd, rectWindow);
		Update(3, dc, CPoint(0, m_Height-BOTTOMHEIGHT), CSize(m_Width, BOTTOMHEIGHT), pBackstageWnd, rectWindow);

		// Clean up
		dc.SelectObject(hOldBitmap);

		DeleteObject(hWindowBitmap);
	}
	else
	{
		// Move behind window
		for (UINT a=0; a<4; a++)
			m_wndShadow[a].SetWindowPos(pBackstageWnd, rectWindow.left+m_wndTopLeft[a].x, rectWindow.top+m_wndTopLeft[a].y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | (Visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	}
}

void CBackstageShadow::Update(CWnd* pBackstageWnd)
{
	ASSERT(IsWindow(m_hWnd));
	ASSERT(pBackstageWnd);

	CRect rectWindow;
	pBackstageWnd->GetWindowRect(rectWindow);

	Update(pBackstageWnd, rectWindow);
}
