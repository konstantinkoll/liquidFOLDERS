
#include "stdafx.h"
#include "StoreWnd.h"
#include "Resource.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"
#include "MainFrm.h"
#include "Migrate.h"


// CPlacesWnd
//

CPlacesWnd::CPlacesWnd()
{
}

CPlacesWnd::~CPlacesWnd()
{
}


BEGIN_MESSAGE_MAP(CPlacesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateCommands)
END_MESSAGE_MAP()

int CPlacesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;
	SetRedraw(FALSE);

	if (m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE) == -1)
		return -1;

	m_wndToolBar.LoadToolBar(ID_PANE_PLACESWND, 0, 0, TRUE);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	SetRedraw(TRUE);
	return 0;
}

void CPlacesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	int heightTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CPlacesWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, afxGlobalData.clrBarFace);

	return TRUE;
}

void CPlacesWnd::OnPaint()
{
//	m_wndList.SetBkColor(afxGlobalData.clrBarFace);
//	m_wndList.SetTextBkColor(afxGlobalData.clrBarFace);

	CDockablePane::OnPaint();
}

void CPlacesWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
