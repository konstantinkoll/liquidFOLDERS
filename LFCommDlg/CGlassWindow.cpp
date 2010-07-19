
// CGlassWindow.cpp: Implementierung der Klasse CGlassWindow
//

#include "stdafx.h"
#include "CGlassWindow.h"


// CGlassWindow
//

CGlassWindow::CGlassWindow()
	: CWnd()
{
	p_App = (LFApplication*)AfxGetApp();
	hTheme = NULL;
	m_IsAeroWindow = FALSE;
}

CGlassWindow::~CGlassWindow()
{
}

void CGlassWindow::UseGlassBackground()
{
	ASSERT(p_App->m_AeroLibLoaded);

	MARGINS margins = { -1 };
	p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &margins);
}


BEGIN_MESSAGE_MAP(CGlassWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
END_MESSAGE_MAP()

int CGlassWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	OnThemeChanged();
	OnCompositionChanged();

	return 0;
}

void CGlassWindow::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CWnd::OnDestroy();
}

LRESULT CGlassWindow::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_WINDOW);
		Invalidate();
	}

	return TRUE;
}

void CGlassWindow::OnCompositionChanged()
{
	if (p_App->m_AeroLibLoaded)
	{
		p_App->zDwmIsCompositionEnabled(&m_IsAeroWindow);

		if (m_IsAeroWindow)
			UseGlassBackground();
	}
	else
	{
		m_IsAeroWindow = FALSE;
	}

	Invalidate();
}
