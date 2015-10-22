
// CGlassPane.cpp: Implementierung der Klasse CGlassPane
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CGlassPane
//

#define GRIPPER     4

CGlassPane::CGlassPane()
	: CWnd()
{
	m_MaxWidth = 128+GRIPPER;
}

BOOL CGlassPane::Create(CWnd* pParentWnd, UINT nID, BOOL IsLeft, INT PreferredWidth)
{
	m_IsLeft = IsLeft;
	m_PreferredWidth = PreferredWidth;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT | WS_EX_NOACTIVATE, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, rect, pParentWnd, nID);
}

void CGlassPane::AdjustLayout()
{
}

INT CGlassPane::GetPreferredWidth()
{
	return m_PreferredWidth;
}

void CGlassPane::SetMaxWidth(INT MaxWidth)
{
	m_MaxWidth = MaxWidth-GRIPPER;
}


BEGIN_MESSAGE_MAP(CGlassPane, CWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CHILDACTIVATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CGlassPane::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
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

LRESULT CGlassPane::OnNcHitTest(CPoint point)
{
	CRect rectWindow;
	GetWindowRect(&rectWindow);
	if (!rectWindow.PtInRect(point))
		return HTNOWHERE;

	CRect rectClient;
	GetClientRect(rectClient);
	ClientToScreen(rectClient);

	return rectClient.PtInRect(point) ? HTCLIENT : m_IsLeft ? HTRIGHT : HTLEFT;
}

void CGlassPane::OnNcPaint()
{
	CWindowDC dc(this);

	CRect rectClient;
	GetClientRect(rectClient);
	ClientToScreen(rectClient);

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);

	if (IsCtrlThemed())
	{
		dc.FillSolidRect(0, 0, GRIPPER, 1, 0xE7E7E7);
		dc.FillSolidRect(0, 1, GRIPPER, 1, 0xF3F3F3);

		ASSERT(GRIPPER==4);
		BYTE Colors[] = { 0xF8, 0xC0, 0xE0, 0xF0 };
		INT Line = rectWindow.Height()*3/5;

		Graphics g(dc);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		for (INT a=0; a<GRIPPER; a++)
		{
			const BYTE clr = Colors[m_IsLeft ? GRIPPER-1-a : a];

			LinearGradientBrush brush1(Point(0, 0), Point(0, Line), Color(0xFF, 0xFF, 0xFF), Color(clr, clr, clr));
			g.FillRectangle(&brush1, a, 2, 1, Line-2);

			LinearGradientBrush brush2(Point(0, Line), Point(0, rectWindow.Height()), Color(clr, clr, clr), Color(0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush2, a, Line, 1, rectWindow.Height()-Line);
		}
	}
	else
	{
		dc.FillSolidRect(rectWindow.left, rectWindow.top, GRIPPER, rectWindow.Height(), GetSysColor(COLOR_3DFACE));
	}
}

BOOL CGlassPane::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	return TRUE;
}

void CGlassPane::OnChildActivate()
{
	GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CGlassPane::OnSize(UINT nType, INT cx, INT cy)
{
	if (GetCapture()==this)
		m_PreferredWidth = cx+GRIPPER;

	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();

	GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CGlassPane::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = 132+GRIPPER;
	lpMMI->ptMaxTrackSize.x = m_MaxWidth+GRIPPER;
}

HBRUSH CGlassPane::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetDCBrushColor(IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));
		hbr = (HBRUSH)GetStockObject(DC_BRUSH);
	}

	return hbr;
}
