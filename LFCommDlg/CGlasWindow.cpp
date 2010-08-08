
// CGlasWindow.cpp: Implementierung der Klasse CGlasWindow
//

#include "stdafx.h"
#include "CGlasWindow.h"


// CGlasWindow
//

CGlasWindow::CGlasWindow()
	: CWnd()
{
	p_App = (LFApplication*)AfxGetApp();
	hTheme = NULL;
	m_IsAeroWindow = FALSE;
	ZeroMemory(&m_Margins, sizeof(MARGINS));
}

CGlasWindow::~CGlasWindow()
{
}

LRESULT CGlasWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_IsAeroWindow)
	{
		LRESULT res;
		if (p_App->zDwmDefWindowProc(m_hWnd, message, wParam, lParam, &res))
			return res;
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}

void CGlasWindow::UseGlasBackground(MARGINS Margins)
{
	m_Margins = Margins;

	if (m_IsAeroWindow)
	{
		Margins.cxLeftWidth += GetSystemMetrics(SM_CXSIZEFRAME);
		Margins.cxRightWidth += GetSystemMetrics(SM_CXSIZEFRAME);
		Margins.cyTopHeight += GetSystemMetrics(SM_CYCAPTION);
		Margins.cyBottomHeight += GetSystemMetrics(SM_CYSIZEFRAME);
		p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &Margins);
	}
}

void CGlasWindow::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	if (m_IsAeroWindow)
	{
		lpRect->top += GetSystemMetrics(SM_CYCAPTION);
		lpRect->left += GetSystemMetrics(SM_CXSIZEFRAME);
		lpRect->right -= GetSystemMetrics(SM_CXSIZEFRAME);
		lpRect->bottom -= GetSystemMetrics(SM_CYSIZEFRAME);
	}
	else
		if (hTheme)
		{
			lpRect->left++;
			lpRect->right--;
		}
}

void CGlasWindow::DrawFrameBackground(CDC* pDC, CRect rect)
{
	if (m_IsAeroWindow)
	{
		pDC->FillSolidRect(rect, 0x000000);
	}
	else
		if (hTheme)
		{
			CRect rframe;
			GetClientRect(rframe);
			rframe.left -= GetSystemMetrics(SM_CXSIZEFRAME);
			rframe.right += GetSystemMetrics(SM_CXSIZEFRAME);
			rframe.bottom += GetSystemMetrics(SM_CYSIZEFRAME);

			p_App->zDrawThemeBackground(hTheme, *pDC, WP_FRAMELEFT, GetActiveWindow()==this ? CS_ACTIVE : CS_INACTIVE, rframe, rect);
		}
		else
		{
			pDC->FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		}
}


BEGIN_MESSAGE_MAP(CGlasWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()

int CGlasWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	OnCompositionChanged();
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

	return 0;
}

void CGlasWindow::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CWnd::OnDestroy();
}

BOOL CGlasWindow::OnEraseBkgnd(CDC* pDC)
{
	CRect rclient;
	GetClientRect(rclient);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rclient.Width();
	dib.bmiHeader.biHeight = -rclient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP bmp = CreateDIBSection(dc.m_hDC, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(bmp);

	if ((m_Margins.cxLeftWidth==-1) || (m_Margins.cxRightWidth==-1) || (m_Margins.cyTopHeight==-1) || (m_Margins.cyBottomHeight==-1))
	{
		DrawFrameBackground(&dc, rclient);
	}
	else
	{
		CRect rectLayout;
		GetLayoutRect(rectLayout);

		CRect rc(rclient);
		rc.right = rectLayout.left = rectLayout.left+m_Margins.cxLeftWidth;
		DrawFrameBackground(&dc, rc);

		rc.CopyRect(rclient);
		rc.left = rectLayout.right = rectLayout.right-m_Margins.cxRightWidth;
		DrawFrameBackground(&dc, rc);

		if (m_Margins.cyTopHeight)
		{
			CRect rc(rclient);
			rc.bottom = rectLayout.top = rectLayout.top+m_Margins.cyTopHeight;
			DrawFrameBackground(&dc, rc);
		}
		if (m_Margins.cyBottomHeight)
		{
			CRect rc(rclient);
			rc.top = rectLayout.bottom = rectLayout.bottom-m_Margins.cyBottomHeight;
			DrawFrameBackground(&dc, rc);
		}

		dc.FillSolidRect(rectLayout, 0xFF0000);
	}

	pDC->BitBlt(0, 0, rclient.Width(), rclient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(bmp);

	return TRUE;
}

LRESULT CGlasWindow::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, m_IsAeroWindow ? _T("CompositedWindow::Window") : VSCLASS_WINDOW);

		if (p_App->zSetWindowThemeAttribute)
		{
			WTA_OPTIONS opt;
			opt.dwMask = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
			opt.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
			p_App->zSetWindowThemeAttribute(m_hWnd, WTA_NONCLIENT, &opt, sizeof(WTA_OPTIONS));
		}
	}

	return TRUE;
}

void CGlasWindow::OnCompositionChanged()
{
	OnThemeChanged();

	if (p_App->m_AeroLibLoaded)
	{
		p_App->zDwmIsCompositionEnabled(&m_IsAeroWindow);
	}
	else
	{
		m_IsAeroWindow = FALSE;
	}
}

void CGlasWindow::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp)
{
	if (m_IsAeroWindow)
	{
		UseGlasBackground(m_Margins);
		return;
	}

	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (hTheme)
	{
		lpncsp->rgrc[0].left--;
		lpncsp->rgrc[0].right++;
	}
}

LRESULT CGlasWindow::OnNcHitTest(CPoint point)
{
	if (m_IsAeroWindow)
	{
		LRESULT res;
		if (p_App->zDwmDefWindowProc(m_hWnd, WM_NCHITTEST, NULL, (point.y<<16) | point.x, &res))
			return res;

		CRect rect;
		GetWindowRect(rect);

		CRect frame;
		AdjustWindowRectEx(frame, GetWindowLong(m_hWnd, GWL_STYLE), FALSE, GetWindowLong(m_hWnd, GWL_EXSTYLE));

		USHORT Row = 1;
		USHORT Col = 1;

		int cx = GetSystemMetrics(SM_CXSIZEFRAME);
		int cy = GetSystemMetrics(SM_CYSIZEFRAME);

		if ((point.y>=rect.top) && (point.y<rect.top+cy))
		{
			Row = 0;
		}
		else
			if ((point.y<rect.bottom) && (point.y>=rect.bottom-cy))
			{
				Row = 2;
			}

		if ((point.x>=rect.left) && (point.x<rect.left+cx))
		{
			Col = 0;
		}
		else
			if ((point.x<rect.right) && (point.x>=rect.right-cx))
			{
				Col = 2;
			}

		const LRESULT HitMatrix[3][3] = 
		{
			{ HTTOPLEFT, HTTOP, HTTOPRIGHT },
			{ HTLEFT, HTCLIENT, HTRIGHT },
			{ HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
		};

		return HitMatrix[Row][Col];
	}

	return CWnd::OnNcHitTest(point);
}

void CGlasWindow::OnActivateApp(BOOL /*bActive*/, DWORD /*dwThreadID*/)
{
	Invalidate();
	UpdateWindow();
}
