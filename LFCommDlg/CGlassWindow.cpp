
// CGlassWindow.cpp: Implementierung der Klasse CGlassWindow
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CGlassWindow
//

CGlassWindow::CGlassWindow()
	: CWnd()
{
	p_App = LFGetApp();
	hAccelerator = NULL;
	hTheme = NULL;
	m_Active = TRUE;
	m_IsAeroWindow = FALSE;
	ZeroMemory(&m_Margins, sizeof(MARGINS));
}

BOOL CGlassWindow::Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, LPCTSTR lpszPlacementPrefix, CSize sz)
{
	m_PlacementPrefix = lpszPlacementPrefix;

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	if ((sz.cx!=0) && (sz.cy!=0))
	{
		rect.left = (rect.left+rect.right)/2 - sz.cx;
		rect.right = rect.left + sz.cx;

		rect.top = (rect.top+rect.bottom)/2 - sz.cy;
		rect.bottom = rect.top + sz.cy;
	}

	if (!CWnd::CreateEx(WS_EX_APPWINDOW | WS_EX_CONTROLPARENT, lpszClassName, lpszWindowName,
		dwStyle | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, NULL, 0))
		return FALSE;

	ZeroMemory(&m_WindowPlacement, sizeof(m_WindowPlacement));
	p_App->GetBinary(m_PlacementPrefix+_T("WindowPlacement"), &m_WindowPlacement, sizeof(m_WindowPlacement));

	if (m_WindowPlacement.length==sizeof(m_WindowPlacement))
	{
		if ((sz.cx!=0) && (sz.cy!=0))
		{
			m_WindowPlacement.rcNormalPosition.right = m_WindowPlacement.rcNormalPosition.left + sz.cx;
			m_WindowPlacement.rcNormalPosition.bottom = m_WindowPlacement.rcNormalPosition.top + sz.cy;
		}

		SetWindowPlacement(&m_WindowPlacement);

		if (IsIconic())
			ShowWindow(SW_RESTORE);
	}

	return TRUE;
}

LRESULT CGlassWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_IsAeroWindow)
	{
		LRESULT res;
		if (p_App->zDwmDefWindowProc(m_hWnd, message, wParam, lParam, &res))
			return res;
	}

	return CWnd::DefWindowProc(message, wParam, lParam);
}

BOOL CGlassWindow::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_TAB))
	{
		CWnd* pWnd = GetNextDlgTabItem(GetFocus(), GetKeyState(VK_SHIFT)<0);
		if (pWnd)
			pWnd->SetFocus();
		return TRUE;
	}

	if (hAccelerator)
		if ((pMsg->message>=WM_KEYFIRST) && (pMsg->message<=WM_KEYLAST))
			if (TranslateAccelerator(m_hWnd, hAccelerator, pMsg))
				return TRUE;

	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CGlassWindow::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return p_App->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CGlassWindow::UseGlasBackground(MARGINS Margins)
{
	m_Margins = Margins;

	if (m_IsAeroWindow)
		p_App->zDwmExtendFrameIntoClientArea(m_hWnd, &Margins);
}

void CGlassWindow::ToggleFullScreen()
{
	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	if (dwStyle & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);

		if (GetWindowPlacement(&m_WindowPlacement) && GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowLong(m_hWnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
			::SetWindowPos(m_hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right-mi.rcMonitor.left, mi.rcMonitor.bottom-mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(&m_WindowPlacement);
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	AdjustLayout();
}

void CGlassWindow::AdjustLayout()
{
}

void CGlassWindow::PostNcDestroy()
{
	delete this;
}

void CGlassWindow::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	if ((!m_IsAeroWindow) && (hTheme))
	{
		lpRect->left++;
		lpRect->right--;
	}
}

void CGlassWindow::DrawFrameBackground(CDC* pDC, CRect rect)
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

UINT CGlassWindow::GetDesign()
{
	return m_IsAeroWindow ? GWD_AERO : hTheme ? GWD_THEMED : GWD_DEFAULT;
}

void CGlassWindow::SetTheme()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), m_IsAeroWindow ? _T("CompositedWindow::Window") : VSCLASS_WINDOW);

		if (p_App->zSetWindowThemeAttribute)
		{
			WTA_OPTIONS opt;
			opt.dwMask = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
			opt.dwFlags = WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON;
			p_App->zSetWindowThemeAttribute(m_hWnd, WTA_NONCLIENT, &opt, sizeof(WTA_OPTIONS));
		}
	}
}


BEGIN_MESSAGE_MAP(CGlassWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_RBUTTONUP()
	ON_WM_INITMENUPOPUP()
	ON_REGISTERED_MESSAGE(LFGetApp()->m_WakeupMsg, OnWakeup)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

INT CGlassWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	p_App->AddFrame(this);

	OnCompositionChanged();
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

	m_Active = (CWnd::GetActiveWindow()==this);

	return 0;
}

void CGlassWindow::OnClose()
{
	if (GetStyle() & WS_OVERLAPPEDWINDOW)
	{
		m_WindowPlacement.length = sizeof(m_WindowPlacement);
		if (!GetWindowPlacement(&m_WindowPlacement))
			goto Skip;
	}

	p_App->WriteBinary(m_PlacementPrefix + _T("WindowPlacement"), (LPBYTE)&m_WindowPlacement, sizeof(m_WindowPlacement));

Skip:
	CWnd::OnClose();
}

void CGlassWindow::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CWnd::OnDestroy();

	p_App->KillFrame(this);
}

BOOL CGlassWindow::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(rectClient);

	if ((m_Margins.cxLeftWidth!=-1) && (m_Margins.cxRightWidth!=-1) && (m_Margins.cyTopHeight!=-1) && (m_Margins.cyBottomHeight!=-1))
	{
		CRect rectLayout;
		GetLayoutRect(rectLayout);

		CRect rc(rectClient);
		rc.right = rectLayout.left = rectLayout.left+m_Margins.cxLeftWidth;
		DrawFrameBackground(pDC, rc);

		rc.CopyRect(rectClient);
		rc.left = rectLayout.right = rectLayout.right-m_Margins.cxRightWidth;
		DrawFrameBackground(pDC, rc);

		if (m_Margins.cyTopHeight)
		{
			CRect rc(rectClient);
			rc.bottom = rectLayout.top = rectLayout.top+m_Margins.cyTopHeight;
			DrawFrameBackground(pDC, rc);
		}
		if (m_Margins.cyBottomHeight)
		{
			CRect rc(rectClient);
			rc.top = rectLayout.bottom = rectLayout.bottom-m_Margins.cyBottomHeight;
			DrawFrameBackground(pDC, rc);
		}

		if ((rectLayout.Width()>0) && (rectLayout.Height()>0))
			pDC->FillSolidRect(rectLayout, 0xFFFFFF);
	}
	else
	{
		DrawFrameBackground(pDC, rectClient);
	}

	return TRUE;
}

void CGlassWindow::OnSysColorChange()
{
	AdjustLayout();
}

LRESULT CGlassWindow::OnThemeChanged()
{
	SetTheme();
	AdjustLayout();

	return TRUE;
}

void CGlassWindow::OnCompositionChanged()
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

LRESULT CGlassWindow::OnDisplayChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (!(GetStyle() & WS_OVERLAPPEDWINDOW))
	{
		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);

		if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi))
			SetWindowPos(&wndTop, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right-mi.rcMonitor.left, mi.rcMonitor.bottom-mi.rcMonitor.top, SWP_NOOWNERZORDER);
	}

	return NULL;
}

void CGlassWindow::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);

	if ((hTheme) && (!m_IsAeroWindow))
	{
		lpncsp->rgrc[0].left--;
		lpncsp->rgrc[0].right++;
	}
}

LRESULT CGlassWindow::OnNcHitTest(CPoint point)
{
	SHORT LButtonDown = GetAsyncKeyState(VK_LBUTTON);
	LRESULT uHitTest = CWnd::OnNcHitTest(point);
	return ((!(GetStyle() & WS_MAXIMIZEBOX)) && (uHitTest>=HTLEFT) && (uHitTest<=HTBOTTOMRIGHT)) ? HTCAPTION : ((uHitTest==HTCLIENT) && (LButtonDown & 0x8000)) ? HTCAPTION : uHitTest;
}

BOOL CGlassWindow::OnNcActivate(BOOL bActive)
{
	if (bActive!=m_Active)
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

	return CWnd::OnNcActivate(bActive);
}

void CGlassWindow::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CGlassWindow::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
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

void CGlassWindow::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	ClientToScreen(&point);
	DWORD Item = GetSystemMenu(FALSE)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
	SendMessage(WM_SYSCOMMAND, (WPARAM)Item);
}

void CGlassWindow::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI state;
	state.m_pMenu = state.m_pParentMenu = pPopupMenu;
	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();

	ASSERT(!state.m_pOther);
	ASSERT(state.m_pMenu);

	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if ((state.m_nID) && (state.m_nID!=(UINT)-1))
			state.DoUpdate(this, FALSE);
	}
}

LRESULT CGlassWindow::OnWakeup(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return 24878;
}

BOOL CGlassWindow::OnCopyData(CWnd* /*pWnd*/, COPYDATASTRUCT* pCopyDataStruct)
{
	if (pCopyDataStruct->cbData!=sizeof(CDS_Wakeup))
		return FALSE;

	CDS_Wakeup cds = *((CDS_Wakeup*)pCopyDataStruct->lpData);
	if (cds.AppID!=p_App->m_AppID)
		return FALSE;

	p_App->OpenCommandLine(cds.Command[0] ? cds.Command : NULL);

	return TRUE;
}
