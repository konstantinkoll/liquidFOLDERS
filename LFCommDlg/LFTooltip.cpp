
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

	CString className = AfxRegisterWndClass(nClassStyle, LFGetApp()->LoadStandardCursor(IDC_ARROW));
	return CWnd::CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, className, _T(""), WS_POPUP, 0, 0, 0, 0, pWndParent->GetSafeHwnd(), NULL);
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

void LFTooltip::Track(CPoint point, HICON hIcon, const CString& strCaption, CString strText)
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
	m_szIcon = CSize(0, 0);
	m_strCaption = strCaption;
	m_strText = strText;
	m_TextHeight = 0;

	if (hIcon)
	{
		ICONINFO IconInfo;
		if (GetIconInfo(hIcon, &IconInfo))
		{
			BITMAP Bitmap;
			if (GetObject(IconInfo.hbmColor, sizeof(Bitmap), &Bitmap))
				m_szIcon = CSize(Bitmap.bmWidth, Bitmap.bmHeight);
		}
	}

	// Size
	CSize Size(0, 0);
	CClientDC dc(this);

	if (!strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		CSize szText = dc.GetTextExtent(strCaption);
		Size.cx = max(Size.cx, szText.cx);
		Size.cy += szText.cy;
		m_TextHeight = max(m_TextHeight, szText.cy);
		dc.SelectObject(pOldFont);

		if (!strText.IsEmpty())
			Size.cy += AFX_TEXT_MARGIN;
	}

	if (!strText.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);

		while (!strText.IsEmpty())
		{
			CString Line;
			INT Pos = strText.Find('\n');
			if (Pos==-1)
			{
				Line = strText;
				strText.Empty();
			}
			else
			{
				Line = strText.Left(Pos);
				strText.Delete(0, Pos+1);
			}

			if (!Line.IsEmpty())
			{
				CSize szText = dc.GetTextExtent(Line);
				Size.cx = max(Size.cx, szText.cx);
				Size.cy += szText.cy;

				m_TextHeight = max(m_TextHeight, szText.cy);
			}
		}

		dc.SelectObject(pOldFont);
	}

	if (hIcon)
	{
		Size.cx += m_szIcon.cx+2*AFX_TEXT_MARGIN;
		Size.cy = max(Size.cy, m_szIcon.cy);
	}

	Size.cx += 2*(AFX_TEXT_MARGIN+3);
	Size.cy += 2*(AFX_TEXT_MARGIN+2)+1;
	if (Size.cx>m_TextHeight*40)
		Size.cx = m_TextHeight*40;

	// Position
	CRect rect;
	rect.top = point.y+18;
	rect.bottom = rect.top+Size.cy;

	if (GetParent()->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rect.left = point.x-Size.cx;
		rect.right = point.x;
	}
	else
	{
		rect.left = point.x;
		rect.right = point.x+Size.cx;
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
			rect.left = rect.right-Size.cx;
		}
		else
			if (rect.left<rectScreen.left)
			{
				rect.left = rectScreen.left;
				rect.right = rect.left+Size.cx;
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
			rect.top = rect.bottom-Size.cy;
		}
		else
			if (rect.top<rectScreen.top)
			{
				rect.top = rectScreen.top;
				rect.bottom = rect.top+Size.cy;
			}

	CRgn rgn;
	m_Themed = IsCtrlThemed();
	m_Flat = m_Themed && (LFGetApp()->OSVersion>=OS_Eight);
	if (m_Themed && !m_Flat)
	{
		rgn.CreateRoundRectRgn(0, 0, Size.cx+1, Size.cy+1, 3, 3);
	}
	else
	{
		rgn.CreateRectRgn(0, 0, Size.cx, Size.cy);
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
	CPaintDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rectClient.Width(), rectClient.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	CRect rect(rectClient);
	rect.DeflateRect(1, 1);

	// Background
	if (m_Flat || (m_Themed && (LFGetApp()->OSVersion==OS_XP)))
	{
		dc.FillSolidRect(rect, 0xFFFFFF);
	}
	else
		if (m_Themed)
		{
			Graphics g(dc);
			LinearGradientBrush brush(Point(0, rect.top), Point(0, rect.bottom+1), Color(0xFF, 0xFF, 0xFF), Color(0xE4, 0xE4, 0xF0));
			g.FillRectangle(&brush, rect.left, rect.top, rect.Width(), rect.Height());
		}
		else
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_INFOBK));
		}

	// Border
	COLORREF clrLine = m_Themed ? 0x767676 : GetSysColor(COLOR_INFOTEXT);
	COLORREF clrText = m_Themed ? m_Flat ? 0x575757 : 0x4C4C4C : GetSysColor(COLOR_INFOTEXT);

	dc.Draw3dRect(rectClient, clrLine, clrLine);

	if (m_Themed && !m_Flat)
	{
		COLORREF clr = (clrLine>>1) | 0x808080;
		dc.SetPixel(rectClient.left+1, rectClient.top+1, clr);
		dc.SetPixel(rectClient.right-2, rectClient.top+1, clr);

		clr = ((clrLine>>1) & 0x7F7F7F) + 0x727278;
		dc.SetPixel(rectClient.left+1, rectClient.bottom-2, clr);
		dc.SetPixel(rectClient.right-2, rectClient.bottom-2, clr);
	}

	// Interior
	rect.DeflateRect(AFX_TEXT_MARGIN+2, AFX_TEXT_MARGIN+1);
	dc.SetTextColor(clrText);

	if (m_Icon)
	{
		DrawIconEx(dc, rect.left, rect.top, m_Icon, m_szIcon.cx, m_szIcon.cy, 0, NULL, DI_NORMAL);
		rect.left += m_szIcon.cx+2*AFX_TEXT_MARGIN;
	}

	if (!m_strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		dc.DrawText(m_strCaption, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
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

			if (!Line.IsEmpty())
			{
				dc.DrawText(Line, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
				rect.top += m_TextHeight;
			}
		}

		dc.SelectObject(pOldFont);
	}

	pDC.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
