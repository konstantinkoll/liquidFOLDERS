
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
}

CGlasWindow::~CGlasWindow()
{
}

void CGlasWindow::UseGlassBackground()
{
	ASSERT(p_App->m_AeroLibLoaded);

	MARGINS margins = { -1 };
	p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &margins);
}


BEGIN_MESSAGE_MAP(CGlasWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
END_MESSAGE_MAP()

int CGlasWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	OnThemeChanged();
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

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_WINDOW);
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
			UseGlassBackground();
	}
	else
	{
		m_IsAeroWindow = FALSE;
	}

	Invalidate();
}
