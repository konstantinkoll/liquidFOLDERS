
// CGlasPane.cpp: Implementierung der Klasse CGlasPane
//

#include "stdafx.h"
#include "CGlasPane.h"


// CGlasPane
//

#define GRIPPER    4

CGlasPane::CGlasPane(BOOL IsLeft)
	: CWnd()
{
	m_IsLeft = IsLeft;
}

BOOL CGlasPane::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CGlasPane, CWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

void CGlasPane::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (m_IsLeft)
	{
		lpncsp->rgrc[0].right -= GRIPPER;
	}
	else
	{
		lpncsp->rgrc[0].left += GRIPPER;
	}
}

LRESULT CGlasPane::OnNcHitTest(CPoint /*point*/)
{
	return m_IsLeft ? HTRIGHT : HTLEFT;
}

void CGlasPane::OnNcPaint()
{
	CWindowDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);
	ClientToScreen(rectClient);

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	rectClient.OffsetRect(-rectWindow.TopLeft());
	rectWindow.OffsetRect(-rectWindow.TopLeft());

	pDC.ExcludeClipRect(rectClient);
	pDC.FillSolidRect(rectWindow, GetSysColor(COLOR_3DFACE));
	pDC.SelectClipRgn(NULL);
}
