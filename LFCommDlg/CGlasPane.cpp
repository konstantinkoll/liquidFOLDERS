
// CGlasPane.cpp: Implementierung der Klasse CGlasPane
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CGlasPane
//

#define GRIPPER    4

CGlasPane::CGlasPane()
	: CWnd()
{
	m_MaxWidth = 128+GRIPPER;
}

BOOL CGlasPane::Create(BOOL IsLeft, INT PreferredWidth, CWnd* pParentWnd, UINT nID)
{
	m_IsLeft = IsLeft;
	m_PreferredWidth = PreferredWidth;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CGlasPane::AdjustLayout()
{
}

INT CGlasPane::GetPreferredWidth()
{
	return m_PreferredWidth;
}

void CGlasPane::SetMaxWidth(INT MaxWidth)
{
	m_MaxWidth = MaxWidth-GRIPPER;
}


BEGIN_MESSAGE_MAP(CGlasPane, CWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CTLCOLOR()
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

LRESULT CGlasPane::OnNcHitTest(CPoint point)
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

BOOL CGlasPane::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	return TRUE;
}

void CGlasPane::OnSize(UINT nType, INT cx, INT cy)
{
	if (GetCapture()==this)
		m_PreferredWidth = cx+GRIPPER;

	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();

	GetParent()->SendMessage(WM_ADJUSTLAYOUT);
}

void CGlasPane::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = 132+GRIPPER;
	lpMMI->ptMaxTrackSize.x = m_MaxWidth+GRIPPER;
}

HBRUSH CGlasPane::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
