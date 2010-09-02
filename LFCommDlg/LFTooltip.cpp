
#include "stdafx.h"
#include "LFTooltip.h"


LFTooltip::LFTooltip()
{
	m_Icon = NULL;
}

BOOL LFTooltip::Create(CWnd* pWndParent)
{
	CString className = AfxRegisterWndClass(CS_DROPSHADOW | CS_SAVEBITS, LoadCursor(NULL, IDC_ARROW));
	return CWnd::CreateEx(0, className, _T(""), WS_POPUP, 0, 0, 0, 0, pWndParent->GetSafeHwnd(), NULL);
}

BOOL LFTooltip::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message>=WM_MOUSEFIRST) && (pMsg->message<=WM_MOUSELAST))
	{
		if (pMsg->message!=WM_MOUSEMOVE)
			Hide();

		CPoint pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		MapWindowPoints(GetParent(), &pt, 1);
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);

		GetParent()->SendMessage(pMsg->message, pMsg->wParam, lParam);
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void LFTooltip::Track(CPoint point, HICON hIcon, CSize szIcon, const CString& strCaption, CString strText)
{
	if (!GetSafeHwnd())
		return;

	if ((m_strText==strText) && (m_strCaption==strCaption))
		return;

	if (IsWindowVisible())
		Hide();
	if (m_Icon)
		DestroyIcon(m_Icon);

	m_Icon = hIcon;
	m_szIcon = szIcon;
	m_strCaption = strCaption;
	m_strText = strText;

	// Size
	CSize sz(0, 0);
	CClientDC dc(this);

	if (!strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		CSize szText = dc.GetTextExtent(strCaption);
		sz.cx = max(sz.cx, szText.cx);
		sz.cy += szText.cy;
		dc.SelectObject(pOldFont);

		if (!strText.IsEmpty())
			sz.cy += AFX_TEXT_MARGIN;
	}

	if (!strText.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);

		while (!strText.IsEmpty())
		{
			CString Line;
			int pos = strText.Find('\n');
			if (pos==-1)
			{
				Line = strText;
				strText.Empty();
			}
			else
			{
				Line = strText.Left(pos);
				strText.Delete(0, pos+1);
			}

			CSize szText = dc.GetTextExtent(Line);
			sz.cx = max(sz.cx, szText.cx);
			sz.cy += szText.cy;
		}

		dc.SelectObject(pOldFont);
	}

	if (hIcon)
	{
		sz.cx += szIcon.cx+2*AFX_TEXT_MARGIN;
		sz.cy = max(sz.cy, szIcon.cy);
	}

	sz.cx += 2*(AFX_TEXT_MARGIN+3);
	sz.cy += 2*(AFX_TEXT_MARGIN+2);

	// Position
	CRect rect;
	rect.top = point.y+18;
	rect.bottom = rect.top+sz.cy;

	if (GetParent()->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rect.left = point.x-sz.cx;
		rect.right = point.x;
	}
	else
	{
		rect.left = point.x;
		rect.right = point.x+sz.cx;
	}

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.Width()>rectScreen.Width())
	{
		rect.left = rectScreen.left;
		rect.right = rectScreen.right;
	}
	else 
		if (rect.right>rectScreen.right)
		{
			rect.right = rectScreen.right;
			rect.left = rect.right-sz.cx;
		}
		else
			if (rect.left<rectScreen.left)
			{
				rect.left = rectScreen.left;
				rect.right = rect.left+sz.cx;
			}

	if (rect.Height()>rectScreen.Height())
	{
		rect.top = rectScreen.top;
		rect.bottom = rectScreen.bottom;
	}
	else
		if (rect.bottom>rectScreen.bottom)
		{
			rect.bottom = point.y-1;
			rect.top = rect.bottom-sz.cy;
		}
		else
			if (rect.top<rectScreen.top)
			{
				rect.top = rectScreen.top;
				rect.bottom = rect.top+sz.cy;
			}

	CMFCToolTipInfo params;
	CMFCVisualManager::GetInstance()->GetToolTipInfo(params);
	if (params.m_bRoundedCorners)
	{
		CRgn rgn;
		rgn.CreateRoundRectRgn(0, 0, sz.cx+1, sz.cy+1, 4, 4);
		SetWindowRgn(rgn, FALSE);
	}

	SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	ShowWindow(SW_SHOWNOACTIVATE);

	Invalidate();
	UpdateWindow();

	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void LFTooltip::Hide()
{
	ShowWindow(SW_HIDE);
}

void LFTooltip::Deactivate()
{
	m_strCaption.Empty();
	m_strText.Empty();
	if (m_Icon)
	{
		DestroyIcon(m_Icon);
		m_Icon = NULL;
	}

	Hide();
}


BEGIN_MESSAGE_MAP(LFTooltip, CWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

void LFTooltip::OnDestroy()
{
	if (m_Icon)
		DestroyIcon(m_Icon);

	CWnd::OnDestroy();
}

BOOL LFTooltip::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void LFTooltip::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect rect;
	GetClientRect(rect);
	rect.DeflateRect(1, 1);

	// Background
	CMFCToolTipInfo params;
	CMFCVisualManager::GetInstance()->GetToolTipInfo(params);

	if (params.m_clrFill==(COLORREF)-1)
	{
		FillRect(dc.GetSafeHdc(), rect, GetSysColorBrush(COLOR_INFOBK));
	}
	else
	{
		if (params.m_clrFillGradient==(COLORREF)-1)
		{
			CBrush br(params.m_clrFill);
			dc.FillRect(rect, &br);
		}
		else
		{
			CDrawingManager dm(dc);
			dm.FillGradient2(rect, params.m_clrFillGradient, params.m_clrFill, (params.m_nGradientAngle==-1) ? 90 : params.m_nGradientAngle);
		}
	}

	// Border
	COLORREF clrLine = (params.m_clrBorder==(COLORREF)-1) ? GetSysColor(COLOR_INFOTEXT) : params.m_clrBorder;
	COLORREF clrText = (params.m_clrText==(COLORREF)-1) ? GetSysColor(COLOR_INFOTEXT) : params.m_clrText;

	CPen penLine(PS_SOLID, 1, clrLine);
	CPen* pOldPen = dc.SelectObject(&penLine);
	rect.InflateRect(1, 1);

	if (!params.m_bRoundedCorners)
	{
		dc.Draw3dRect(rect, clrLine, clrLine);
	}
	else
	{
		const int nOffset = 2;

		dc.MoveTo(rect.left+nOffset, rect.top);
		dc.LineTo(rect.right-nOffset-1, rect.top);

		dc.LineTo(rect.right-1, rect.top+nOffset);
		dc.LineTo(rect.right-1, rect.bottom-1-nOffset);

		dc.LineTo(rect.right-nOffset-1, rect.bottom-1);
		dc.LineTo(rect.left+nOffset, rect.bottom-1);

		dc.LineTo(rect.left, rect.bottom-1-nOffset);
		dc.LineTo(rect.left, rect.top+nOffset);

		dc.LineTo(rect.left+nOffset, rect.top);
	}

	dc.SelectObject(pOldPen);

	// Interior
	rect.DeflateRect(AFX_TEXT_MARGIN+3, AFX_TEXT_MARGIN+2);
	dc.SetTextColor(clrText);

	if (m_Icon)
	{
		DrawIconEx(dc, rect.left, rect.top, m_Icon, m_szIcon.cx, m_szIcon.cy, 0, NULL, DI_NORMAL);
		rect.left += m_szIcon.cx+2*AFX_TEXT_MARGIN;
	}

	if (!m_strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		dc.DrawText(m_strCaption, rect, DT_LEFT | DT_SINGLELINE);
		rect.top += dc.GetTextExtent(m_strCaption).cy+AFX_TEXT_MARGIN;
		dc.SelectObject(pOldFont);
	}

	if (!m_strText.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
		dc.DrawText(m_strText, rect, DT_LEFT);
		dc.SelectObject(pOldFont);
	}
}
