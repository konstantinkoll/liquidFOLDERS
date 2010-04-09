
#pragma once
#include "stdafx.h"
#include "StoreWnd.h"
#include "Resource.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"
#include "MainFrm.h"
#include "Migrate.h"


// CWorkflowWnd
//

CWorkflowWnd::CWorkflowWnd()
{
}

CWorkflowWnd::~CWorkflowWnd()
{
}


BEGIN_MESSAGE_MAP(CWorkflowWnd, LFCaptionBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ITEMCHANGING, 1, OnItemChanging)
	ON_NOTIFY(LVN_ITEMACTIVATE, 1, OnItemActivate)
END_MESSAGE_MAP()

int CWorkflowWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (LFCaptionBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	SetRedraw(FALSE);

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_NOCOLUMNHEADER | LVS_SINGLESEL;
	if (!m_wndList.Create(dwViewStyle, CRect(0, 0, 32767, 0), this, 1))
		return -1;
	
	const DWORD dwExStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_ONECLICKACTIVATE;
	m_wndList.SetExtendedStyle(dwExStyle);
	m_wndList.SetBkColor(afxGlobalData.clrBarFace);
	m_wndList.SetTextBkColor(afxGlobalData.clrBarFace);
	m_wndList.SetView(LV_VIEW_ICON);
	m_wndList.SetIconSpacing(192, 96);

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	for (UINT a=0; a<4; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_TILE0+a));
		lvi.pszText = tmpStr.GetBuffer();
		lvi.iItem = a;
		lvi.iImage = a;
		lvi.stateMask = LVIS_CUT;
		lvi.state = (a%2) ? LVIS_CUT : 0;

		m_wndList.InsertItem(&lvi);
	}

	// Load icons
	m_Icons.Create(128, 64, ILC_COLOR32, 4, 1);
	for (UINT a=0; a<4; a++)
	{
		HICON ic = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TILE0+a), IMAGE_ICON, 128, 64, LR_LOADTRANSPARENT);
		m_Icons.Add(ic);
		DestroyIcon(ic);
	}
	m_wndList.SetImageList(&m_Icons, LVSIL_NORMAL);

	SetRedraw(TRUE);
	return 0;
}

void CWorkflowWnd::OnSize(UINT nType, int cx, int cy)
{
	LFCaptionBar::OnSize(nType, cx, cy);

	m_wndList.SetWindowPos(NULL, m_rectInfo.left+1, m_rectInfo.top+1, m_rectInfo.Width()-2, m_rectInfo.Height()-2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CWorkflowWnd::OnSetFocus(CWnd* pOldWnd)
{
	LFCaptionBar::OnSetFocus(pOldWnd);
	m_wndList.SetFocus();
}

void CWorkflowWnd::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW* pNMListView = (NMLISTVIEW*)pNMHDR;
	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		*pResult = m_wndList.GetItemState(pNMListView->iItem, LVIS_CUT);
	}
	else
	{
		*pResult = FALSE;
	}
}

void CWorkflowWnd::OnItemActivate(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int idx = m_wndList.GetNextItem(-1, LVIS_SELECTED);

	switch (idx)
	{
	case 0:
		GetParentFrame()->PostMessage(WM_COMMAND, ID_APP_SHOWPLACES);
		break;
	case 1:
		GetParentFrame()->PostMessage(WM_COMMAND, ID_APP_FOCUSMAIN);
		break;
	case 2:
		GetParentFrame()->PostMessage(WM_COMMAND, ID_APP_SHOWSTORES);
		break;
	case 3:
		GetParentFrame()->PostMessage(WM_COMMAND, ID_MIGRATE_START);
		break;
	}

	*pResult = 0;
}
