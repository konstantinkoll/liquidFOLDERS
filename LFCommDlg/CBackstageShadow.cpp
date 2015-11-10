
// CBackstageShadow.cpp: Implementierung der Klasse CBackstageShadow
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageShadow
//

#define SHADOWSIZE       20
#define SHADOWOFFSET     SHADOWSIZE/2-2

CBackstageShadow::CBackstageShadow()
	: CWnd()
{
	m_Width = m_Height = 0;
}

BOOL CBackstageShadow::Create()
{
	CString className = AfxRegisterWndClass(0);

	return CWnd::CreateEx(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT, className, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL);
}

void CBackstageShadow::Update(CWnd* pBackstageWnd, CRect rectWindow)
{
	ASSERT(IsWindow(m_hWnd));
	ASSERT(pBackstageWnd);

	rectWindow.InflateRect(SHADOWSIZE+1, SHADOWSIZE+1);
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

			Pen pen(Color((BYTE)(((a+3)*(a+3)*(a+3)>>6)), 0x00, 0x00, 0x00));
			g.DrawPath(&pen, &path);

			rect.DeflateRect(1, 1);
		}

		rect.top -= SHADOWOFFSET;
		CreateRoundRectangle(rect, BACKSTAGERADIUS, path);

		Pen pen(Color(0x00, 0x00, 0x00));
		g.DrawPath(&pen, &path);

		// Update system-managed bitmap of window
		POINT ptDst = { rectWindow.left, rectWindow.top };
		SIZE szWindow = { rectWindow.Width(), rectWindow.Height() };
		POINT ptSrc = { 0, 0 };
		UpdateLayeredWindow(&dc, &ptDst, &szWindow, &dc, &ptSrc, 0x000000, &BF, ULW_ALPHA);

		// Clean up
		dc.SelectObject(hOldBitmap);
		dc.DeleteDC();

		DeleteObject(hWindowBitmap);

		m_Width = rectWindow.Width();
		m_Height = rectWindow.Height();
	}

	// Behind window
	SetWindowPos(pBackstageWnd, rectWindow.left, rectWindow.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | (Visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}

void CBackstageShadow::Update(CWnd* pBackstageWnd)
{
	ASSERT(IsWindow(m_hWnd));
	ASSERT(pBackstageWnd);

	CRect rectWindow;
	pBackstageWnd->GetWindowRect(rectWindow);

	Update(pBackstageWnd, rectWindow);
}
