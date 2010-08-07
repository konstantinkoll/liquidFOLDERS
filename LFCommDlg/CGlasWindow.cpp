
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
		p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &Margins);
}

void CGlasWindow::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	if (m_IsAeroWindow)
	{
		lpRect->top += GetSystemMetrics(SM_CYCAPTION);
	}
	else
		if (hTheme)
		{
			lpRect->left++;
			lpRect->right--;
		}
}


BEGIN_MESSAGE_MAP(CGlasWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
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

	return 0;
}

void CGlasWindow::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CWnd::OnDestroy();
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

		Invalidate();
	}

	return TRUE;
}

void CGlasWindow::OnCompositionChanged()
{
	if (p_App->m_AeroLibLoaded)
	{
		p_App->zDwmIsCompositionEnabled(&m_IsAeroWindow);

		if (m_IsAeroWindow)
			p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &m_Margins);
	}
	else
	{
		m_IsAeroWindow = FALSE;
	}

	OnThemeChanged();
	Invalidate();
}

void CGlasWindow::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp)
{
	if (m_IsAeroWindow)
	{
		if ((m_Margins.cxLeftWidth!=-1) && (m_Margins.cxRightWidth!=-1) && (m_Margins.cyTopHeight!=-1) && (m_Margins.cyBottomHeight!=-1))
		{
			lpncsp->rgrc[0].left += m_Margins.cxLeftWidth;
			lpncsp->rgrc[0].right -= m_Margins.cxRightWidth;
			lpncsp->rgrc[0].bottom -= m_Margins.cyBottomHeight;
		}
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

		res = CWnd::OnNcHitTest(point);
		if (res==HTCLIENT)
		{
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

			res = (GetStyle() & WS_SIZEBOX) ? HitMatrix[Row][Col] : HTCLIENT;
		}

		return res;
	}

	return CWnd::OnNcHitTest(point);
}

void CGlasWindow::OnActivateApp(BOOL /*bActive*/, DWORD /*dwThreadID*/)
{
	Invalidate();
	UpdateWindow();
}
