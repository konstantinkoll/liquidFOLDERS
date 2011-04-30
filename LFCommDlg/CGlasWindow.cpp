
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
	p_PopupWindow = NULL;
	hTheme = NULL;
	m_Active = TRUE;
	m_IsAeroWindow = FALSE;
	ZeroMemory(&m_Margins, sizeof(MARGINS));
}

BOOL CGlasWindow::Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::CreateEx(WS_EX_APPWINDOW | WS_EX_CONTROLPARENT, lpszClassName, lpszWindowName,
		dwStyle | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pParentWnd, nID);
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

BOOL CGlasWindow::PreTranslateMessage(MSG* pMsg)
{
	if (p_PopupWindow)
		if (IsWindow(p_PopupWindow->GetSafeHwnd()))
			if (GetCapture()!=p_PopupWindow->GetOwner())
			{
				CRect rect;
				p_PopupWindow->GetClientRect(rect);
				p_PopupWindow->ClientToScreen(rect);

				CPoint pt;
				GetCursorPos(&pt);

				switch (pMsg->message)
				{
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MBUTTONUP:
				case WM_NCLBUTTONDOWN:
				case WM_NCRBUTTONDOWN:
				case WM_NCMBUTTONDOWN:
				case WM_NCLBUTTONUP:
				case WM_NCRBUTTONUP:
				case WM_NCMBUTTONUP:
					if (!rect.PtInRect(pt))
					{
						p_PopupWindow->GetOwner()->SendMessage(WM_CLOSEDROPDOWN);
						return TRUE;
					}
				}
			}

	if ((pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_TAB))
	{
		CWnd* pWnd = GetNextDlgTabItem(GetFocus(), GetKeyState(VK_SHIFT)<0);
		if (pWnd)
			pWnd->SetFocus();
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CGlasWindow::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return p_App->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CGlasWindow::UseGlasBackground(MARGINS Margins)
{
	m_Margins = Margins;

	if (m_IsAeroWindow)
		p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &Margins);
}

void CGlasWindow::AdjustLayout()
{
}

void CGlasWindow::PostNcDestroy()
{
	delete this;
}

void CGlasWindow::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	if ((!m_IsAeroWindow) && (hTheme))
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

			p_App->zDrawThemeBackground(hTheme, *pDC, WP_FRAMELEFT, m_Active ? CS_ACTIVE : CS_INACTIVE, rframe, rect);
		}
		else
		{
			pDC->FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		}
}

UINT CGlasWindow::GetDesign()
{
	return m_IsAeroWindow ? GWD_AERO : hTheme ? GWD_THEMED : GWD_DEFAULT;
}

void CGlasWindow::SetTheme()
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
}

CWnd* CGlasWindow::RegisterPopupWindow(CWnd* pPopupWnd)
{
	CWnd* old = p_PopupWindow;
	p_PopupWindow = pPopupWnd;
	return old;
}


BEGIN_MESSAGE_MAP(CGlasWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_ACTIVATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_RBUTTONUP()
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

INT CGlasWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	OnCompositionChanged();
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

	m_Active = (CWnd::GetActiveWindow()==this);

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

	if ((m_Margins.cxLeftWidth!=-1) && (m_Margins.cxRightWidth!=-1) && (m_Margins.cyTopHeight!=-1) && (m_Margins.cyBottomHeight!=-1))
	{
		CRect rectLayout;
		GetLayoutRect(rectLayout);

		CRect rc(rclient);
		rc.right = rectLayout.left = rectLayout.left+m_Margins.cxLeftWidth;
		DrawFrameBackground(pDC, rc);

		rc.CopyRect(rclient);
		rc.left = rectLayout.right = rectLayout.right-m_Margins.cxRightWidth;
		DrawFrameBackground(pDC, rc);

		if (m_Margins.cyTopHeight)
		{
			CRect rc(rclient);
			rc.bottom = rectLayout.top = rectLayout.top+m_Margins.cyTopHeight;
			DrawFrameBackground(pDC, rc);
		}
		if (m_Margins.cyBottomHeight)
		{
			CRect rc(rclient);
			rc.top = rectLayout.bottom = rectLayout.bottom-m_Margins.cyBottomHeight;
			DrawFrameBackground(pDC, rc);
		}

		if ((rectLayout.Width()>0) && (rectLayout.Height()>0))
			pDC->FillSolidRect(rectLayout, 0xFFFFFF);
	}
	else
	{
		DrawFrameBackground(pDC, rclient);
	}

	return TRUE;
}

void CGlasWindow::OnSysColorChange()
{
	AdjustLayout();
}

LRESULT CGlasWindow::OnThemeChanged()
{
	SetTheme();
	AdjustLayout();

	return TRUE;
}

void CGlasWindow::OnCompositionChanged()
{
	SetTheme();

	if (p_App->m_AeroLibLoaded)
	{
		p_App->zDwmIsCompositionEnabled(&m_IsAeroWindow);
		UseGlasBackground(m_Margins);
	}
	else
	{
		m_IsAeroWindow = FALSE;
	}

	AdjustLayout();
}

void CGlasWindow::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if ((hTheme) && (!m_IsAeroWindow))
	{
		lpncsp->rgrc[0].left--;
		lpncsp->rgrc[0].right++;
	}
}

LRESULT CGlasWindow::OnNcHitTest(CPoint point)
{
	SHORT LButtonDown = GetAsyncKeyState(VK_LBUTTON);
	LRESULT uHitTest = CWnd::OnNcHitTest(point);
	return ((!(GetStyle() & WS_MAXIMIZEBOX)) && (uHitTest>=HTLEFT) && (uHitTest<=HTBOTTOMRIGHT)) ? HTCAPTION : ((uHitTest==HTCLIENT) && (LButtonDown & 0x8000)) ? HTCAPTION : uHitTest;
}

BOOL CGlasWindow::OnNcActivate(BOOL bActive)
{
	if ((bActive!=m_Active) && ((!p_PopupWindow) || (bActive)))
	{
		m_Active = bActive;

		if (GetDesign()==GWD_THEMED)
		{
			Invalidate();

			for (POSITION p=m_GlasChildren.GetHeadPosition(); p; )
				m_GlasChildren.GetNext(p)->RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);

			UpdateWindow();
		}
	}

	return (bActive || !p_PopupWindow) ? CWnd::OnNcActivate(bActive) : TRUE;
}

void CGlasWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (!p_PopupWindow)
		CWnd::OnActivate(nState, pWndOther, bMinimized);
}


void CGlasWindow::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CGlasWindow::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CWnd::OnGetMinMaxInfo(lpMMI);

	if (GetStyle() & WS_MAXIMIZEBOX)
	{
		lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 
			384+
			((m_Margins.cxLeftWidth>0) ? m_Margins.cxLeftWidth : 0)+
			((m_Margins.cxRightWidth>0) ? m_Margins.cxRightWidth : 0));
		lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 
			256+GetSystemMetrics(SM_CYCAPTION)+
			((m_Margins.cyTopHeight>0) ? m_Margins.cyTopHeight : 0)+
			((m_Margins.cyBottomHeight>0) ? m_Margins.cyBottomHeight : 0));
	}
}

void CGlasWindow::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	ClientToScreen(&point);
	DWORD Item = GetSystemMenu(FALSE)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
	SendMessage(WM_SYSCOMMAND, (WPARAM)Item);
}

void CGlasWindow::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther==NULL);
	ASSERT(state.m_pParentMenu==NULL);

	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu==pPopupMenu->m_hMenu)
	{
		state.m_pParentMenu = pPopupMenu;
	}
	else
	{
		hParentMenu = ::GetMenu(m_hWnd);
		if (hParentMenu)
		{
			INT nIndexMax = GetMenuItemCount(hParentMenu);
			for (INT nIndex=0; nIndex<nIndexMax; nIndex++)
				if (GetSubMenu(hParentMenu, nIndex)==pPopupMenu->m_hMenu)
				{
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (!state.m_nID)
			continue;

		ASSERT(!state.m_pOther);
		ASSERT(state.m_pMenu);
		if (state.m_nID ==(UINT)-1)
		{
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if ((!state.m_pSubMenu) || ((state.m_nID=state.m_pSubMenu->GetMenuItemID(0))== 0) || (state.m_nID==(UINT)-1))
				continue;

			state.DoUpdate(this, TRUE);
		}
		else
		{
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount<state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax-nCount);
			while ((state.m_nIndex<nCount) && (pPopupMenu->GetMenuItemID(state.m_nIndex)==state.m_nID))
				state.m_nIndex++;
		}
		state.m_nIndexMax = nCount;
	}
}
