
// CDropdownSelector.cpp: Implementierung der Klasse CDropdownSelector
//

#include "stdafx.h"
#include "CDropdownSelector.h"
#include "LFCommDlg.h"


// CDropdownSelector
//

#define BORDER          4

CDropdownSelector::CDropdownSelector()
	: CWnd()
{
	p_App = (LFApplication*)AfxGetApp();
	hTheme = NULL;
	m_Icon = NULL;
	m_IsEmpty = TRUE;
	m_Hover = FALSE;
}

BOOL CDropdownSelector::Create(CString EmptyHint, CGlasWindow* pParentWnd, UINT nID)
{
	m_EmptyHint = EmptyHint;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T("DropdownSelector"), dwStyle, rect, pParentWnd, nID);
}

void CDropdownSelector::SetEmpty(BOOL Repaint)
{
	m_IsEmpty = TRUE;

	if (m_Icon)
	{
		DestroyIcon(m_Icon);
		m_Icon = NULL;
	}

	if (Repaint)
		Invalidate();
}

void CDropdownSelector::SetItem(CString Caption, HICON hIcon, CString DisplayName, BOOL Repaint)
{
	m_Caption = Caption;
	m_Icon = hIcon;
	m_DisplayName = DisplayName;
	m_IsEmpty = FALSE;

	if (Repaint)
		Invalidate();
}

UINT CDropdownSelector::GetPreferredHeight()
{
	LOGFONT lf;
	p_App->m_DefaultFont.GetLogFont(&lf);
	UINT h = max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON));

	return h+2*BORDER;
}


BEGIN_MESSAGE_MAP(CDropdownSelector, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

int CDropdownSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	OnThemeChanged();

	return 0;
}

void CDropdownSelector::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);
	if (m_Icon)
		DestroyIcon(m_Icon);

	CWnd::OnDestroy();
}

BOOL CDropdownSelector::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CDropdownSelector::OnPaint()
{
	CPaintDC pDC(this);

	CRect rclient;
	GetClientRect(rclient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rclient.Width();
	dib.bmiHeader.biHeight = -rclient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP bmp = CreateDIBSection(dc.m_hDC, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(bmp);

	Graphics g(dc);

	CGlasWindow* pCtrlSite = (CGlasWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rclient);
	const BYTE Alpha = m_Hover ? 0xF0 : 0xD0;

	CRect rcontent(rclient);
	switch (pCtrlSite->GetDesign())
	{
	case GWD_AERO:
	case GWD_THEMED:
		{
			CRect rbounds(rcontent);
			rbounds.right--;
			rbounds.bottom--;

			rcontent.DeflateRect(2, 2);
			SolidBrush brush1(Color(Alpha, 0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush1, rcontent.left, rcontent.top, rcontent.Width(), rcontent.Height());
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			GraphicsPath path;
			CreateRoundRectangle(rbounds, 2, path);
			Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
			g.DrawPath(&pen, &path);
			rbounds.DeflateRect(1, 1);

			CreateRoundRectangle(rbounds, 1, path);
			LinearGradientBrush brush2(Point(0, rbounds.top), Point(0, rbounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
			pen.SetBrush(&brush2);
			g.DrawPath(&pen, &path);

			break;
		}
	case GWD_DEFAULT:
		{
			rcontent.DeflateRect(1, 1);
			dc.Draw3dRect(rcontent, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
			rcontent.DeflateRect(1, 1);
			break;
		}
	}

	CRect rclip(rcontent);
	rclip.left = rclip.right-GetSystemMetrics(SM_CXHSCROLL);
	CRect rarrow(rclip);

	if (hTheme)
	{
		// Hack to achive the same style as Windows Explorer
		if (p_App->OSVersion>OS_XP)
		{
			rarrow.InflateRect(1, 1);
			if (m_Hover)
			{
				rclip.left--;
			}
			else
			{
				rarrow.InflateRect(1, 1);
			}
		}

		p_App->zDrawThemeBackground(hTheme, dc, CP_DROPDOWNBUTTON, m_Hover ? CBXS_HOT : CBXS_NORMAL, rarrow, rclip);
	}
	else
	{
		if (m_Hover)
			dc.DrawFrameControl(rarrow, DFC_BUTTON, DFCS_TRANSPARENT | 16 | DFCS_HOT);

		dc.DrawFrameControl(rarrow, DFC_MENU, DFCS_TRANSPARENT | 16 | (m_Hover ? DFCS_HOT : DFCS_FLAT));
	}

	CRect rtext(rcontent);
	rtext.right = rclip.left;
	rtext.DeflateRect(BORDER, 0);

	CFont* pOldFont;

	if (m_IsEmpty)
	{
		pOldFont = dc.SelectObject(&p_App->m_ItalicFont);
		dc.SetTextColor(0x808080);
		dc.DrawText(m_EmptyHint, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		pOldFont = dc.SelectObject(&p_App->m_DefaultFont);
		COLORREF c1 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000;
		COLORREF c2 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_3DSHADOW) : 0x808080;

		if (!m_Caption.IsEmpty())
		{
			dc.SetTextColor(m_Hover ? c1 : c2);
			dc.DrawText(m_Caption, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
			rtext.left += dc.GetTextExtent(m_Caption, m_Caption.GetLength()).cx+BORDER;
		}

		if (m_Icon)
		{
			int cx = GetSystemMetrics(SM_CXSMICON);
			int cy = GetSystemMetrics(SM_CYSMICON);

			if (rtext.left+cx<rtext.right)
			{
				DrawIconEx(dc, rtext.left, rtext.top+(rtext.Height()-cy)/2, m_Icon, cx, cy, 0, NULL, DI_NORMAL);
				rtext.left += cx+BORDER;
			}
		}

		if (!m_DisplayName.IsEmpty())
		{
			dc.SetTextColor(c1);
			dc.DrawText(m_DisplayName, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
		}
	}

	dc.SelectObject(pOldFont);

	// Set alpha
	BITMAP bm;
	GetObject(bmp, sizeof(BITMAP), &bm);
	BYTE* pBits = ((BYTE*)bm.bmBits)+4*(rcontent.top*rclient.Width()+rcontent.left);
	for (int row=rcontent.top; row<rcontent.bottom; row++)
	{
		for (int col=rcontent.left; col<rcontent.right; col++)
		{
			*(pBits+3) = Alpha;
			pBits += 4;
		}
		pBits += 4*(rclient.Width()-rcontent.Width());
	}

	pDC.BitBlt(0, 0, rclient.Width(), rclient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(bmp);
}

LRESULT CDropdownSelector::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, _T("ComboBox"));
	}

	return TRUE;
}

void CDropdownSelector::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CDropdownSelector::OnMouseLeave()
{
	m_Hover = FALSE;
	Invalidate();

	CWnd::OnMouseLeave();
}
