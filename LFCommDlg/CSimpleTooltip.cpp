
#include "stdafx.h"
#include "CSimpleTooltip.h"


CSimpleTooltip::CSimpleTooltip()
{
	pWndParent = NULL;
	m_rect.SetRectEmpty();
}

CSimpleTooltip::~CSimpleTooltip()
{
}

BOOL CSimpleTooltip::Create(CWnd* _pWndParent)
{
	pWndParent = _pWndParent;

	CString className = AfxRegisterWndClass(CS_DROPSHADOW | CS_SAVEBITS, NULL, NULL);
	return CWnd::CreateEx(0, className, _T(""), WS_POPUP, 0, 0, 0, 0, _pWndParent->GetSafeHwnd(), (HMENU)NULL);
}

BOOL CSimpleTooltip::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message>=WM_MOUSEFIRST) && (pMsg->message<=WM_MOUSELAST))
	{
		if (pMsg->message != WM_MOUSEMOVE)
			Hide();

		CPoint pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		MapWindowPoints(pWndParent, &pt, 1);
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);

		pWndParent->SendMessage(pMsg->message, pMsg->wParam, lParam);
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CSimpleTooltip::Track(CRect rect, const CString& strText)
{
	if (!GetSafeHwnd())
		return;
	if ((m_rect==rect) && (m_strText==strText))
		return;

	m_rect = rect;
	m_strText = strText;

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);

	CSize sz = dc.GetTextExtent(m_strText);
	sz.cx += 2*AFX_TEXT_MARGIN+4;
	sz.cy += 2*AFX_TEXT_MARGIN+4;

	dc.SelectObject(pOldFont);

	if (pWndParent->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rect.left = rect.right-sz.cx;
	}
	else
	{
		rect.right = rect.left+sz.cx;
	}
	rect.bottom = rect.top+sz.cy;

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
			rect.bottom = rectScreen.bottom;
			rect.top = rect.bottom-sz.cy;
		}
		else
			if (rect.top<rectScreen.top)
			{
				rect.top = rectScreen.top;
				rect.bottom = rect.bottom+sz.cy;
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

void CSimpleTooltip::Hide()
{
	if (GetSafeHwnd())
		ShowWindow(SW_HIDE);
}

void CSimpleTooltip::Deactivate()
{
	m_strText.Empty();
	m_rect.SetRectEmpty();

	Hide();
}


BEGIN_MESSAGE_MAP(CSimpleTooltip, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CSimpleTooltip::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CSimpleTooltip::OnPaint()
{
	CPaintDC dc(this);
	dc.SetBkMode(TRANSPARENT);

	CRect rect;
	GetClientRect(rect);
	rect.DeflateRect(1, 1);

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

	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
	dc.SetTextColor(clrText);
	dc.DrawText(m_strText, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldPen);
}
