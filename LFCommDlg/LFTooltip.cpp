
// LFTooltip.cpp: Implementierung der Klasse LFTooltip
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFTooltip
//

LFTooltip::LFTooltip()
	: CWnd()
{
	m_Icon = NULL;
}

BOOL LFTooltip::Create(CWnd* pWndParent)
{
	UINT nClassStyle = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
	BOOL bDropShadow;
	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &bDropShadow, FALSE);
	if (bDropShadow)
		nClassStyle |= CS_DROPSHADOW;

	CString className = AfxRegisterWndClass(nClassStyle, LoadCursor(NULL, IDC_ARROW));
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
		m_TextHeight = 0;

		while (!strText.IsEmpty())
		{
			CString Line;
			INT pos = strText.Find('\n');
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

			m_TextHeight = max(m_TextHeight, szText.cy);
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
	if (sz.cx>m_TextHeight*25)
		sz.cx = m_TextHeight*25;

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

	CRgn rgn;
	m_Themed = IsCtrlThemed();
	if (m_Themed)
	{
		rgn.CreateRoundRectRgn(0, 0, sz.cx+1, sz.cy+1, 4, 4);
	}
	else
	{
		rgn.CreateRectRgn(0, 0, sz.cx, sz.cy);
	}
	SetWindowRgn(rgn, FALSE);

	SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	ShowWindow(SW_SHOWNOACTIVATE);

	Invalidate();
	UpdateWindow();
}

void LFTooltip::Hide()
{
	if (IsWindow(m_hWnd))
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
	if (m_Themed)
	{
		Graphics g(dc);
		LinearGradientBrush brush(Point(0, rect.top), Point(0, rect.bottom+1), Color(0xFF, 0xFF, 0xFF), Color(0xE4, 0xE5, 0xF0));
		g.FillRectangle(&brush, rect.left, rect.top, rect.Width(), rect.Height());
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_INFOBK));
	}

	// Border
	COLORREF clrLine = m_Themed ? 0x767676 : GetSysColor(COLOR_INFOTEXT);
	COLORREF clrText = m_Themed ? 0x4C4C4C : GetSysColor(COLOR_INFOTEXT);

	CPen penLine(PS_SOLID, 1, clrLine);
	CPen* pOldPen = dc.SelectObject(&penLine);
	rect.InflateRect(1, 1);

	if (m_Themed)
	{
		const INT nOffset = 2;

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
	else
	{
		dc.Draw3dRect(rect, clrLine, clrLine);
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
		dc.DrawText(m_strCaption, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
		rect.top += dc.GetTextExtent(m_strCaption).cy+AFX_TEXT_MARGIN;
		dc.SelectObject(pOldFont);
	}

	if (!m_strText.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
		CString strText = m_strText;

		while (!strText.IsEmpty())
		{
			CString Line;
			INT pos = strText.Find('\n');
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

			dc.DrawText(Line, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
			rect.top += m_TextHeight;
		}

		dc.SelectObject(pOldFont);
	}
}
