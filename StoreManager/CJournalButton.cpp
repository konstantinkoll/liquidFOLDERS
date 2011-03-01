
// CJournalButton.cpp: Implementierung der Klasse CJournalButton
//

#include "stdafx.h"
#include "CJournalButton.h"
#include "Resource.h"


// CJournalButton
//

#define XOffset (m_IsLarge ? 0 : 1)
#define YOffset (m_IsLarge ? 0 : -1)

CJournalButton::CJournalButton()
	: CWnd()
{
}

BOOL CJournalButton::Create(UINT SuggestedHeight, CGlasWindow* pParentWnd, UINT nID)
{
	m_IsLarge = FALSE;//(SuggestedHeight>=32);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

UINT CJournalButton::GetPreferredHeight()
{
	return m_Frame.m_pBitmap->GetHeight();
}

UINT CJournalButton::GetPreferredWidth()
{
	return m_Frame.m_pBitmap->GetWidth();
}

void CJournalButton::DrawLeft(Graphics& g, CGdiPlusBitmap* pGdiPlusBitmap)
{
	Bitmap* pBmp = pGdiPlusBitmap->m_pBitmap;
	const UINT Width = pBmp->GetWidth();
	const UINT Height = pBmp->GetHeight();

	g.DrawImage(pBmp, XOffset, YOffset, 0, 0, Width/2, Height, UnitPixel);
}

void CJournalButton::DrawRight(Graphics& g, CGdiPlusBitmap* pGdiPlusBitmap)
{
	Bitmap* pBmp = pGdiPlusBitmap->m_pBitmap;
	const UINT Width = pBmp->GetWidth();
	const UINT Height = pBmp->GetHeight();

	g.DrawImage(pBmp, XOffset+Width/2+1, YOffset, Width/2, 0, Width-Width/2, Height, UnitPixel);
}


BEGIN_MESSAGE_MAP(CJournalButton, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

INT CJournalButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_Frame.Load(m_IsLarge ? IDB_JOURNAL_FRAME1 : IDB_JOURNAL_FRAME0, _T("PNG"));
	m_Normal.Load(m_IsLarge ? IDB_JOURNAL_NORMAL1 : IDB_JOURNAL_NORMAL0, _T("PNG"));
	m_Hot.Load(m_IsLarge ? IDB_JOURNAL_HOT1 : IDB_JOURNAL_HOT0, _T("PNG"));
	m_Pressed.Load(m_IsLarge ? IDB_JOURNAL_PRESSED1 : IDB_JOURNAL_PRESSED0, _T("PNG"));
	m_Disabled.Load(m_IsLarge ? IDB_JOURNAL_DISABLED1 : IDB_JOURNAL_DISABLED0, _T("PNG"));

	return 0;
}

void CJournalButton::OnDestroy()
{
	CWnd::OnDestroy();
}

BOOL CJournalButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CJournalButton::OnPaint()
{
	CPaintDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rectClient.Width();
	dib.bmiHeader.biHeight = -rectClient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmp);

	Graphics g(dc);

	CGlasWindow* pCtrlSite = (CGlasWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);

	g.DrawImage(m_Frame.m_pBitmap, 0, 0);

	DrawLeft(g, &m_Disabled);
	DrawRight(g, &m_Normal);
//	g.DrawImage(, m_IsLarge ? 0 : 1, m_IsLarge ? 0 : -1);

	/*const BYTE Alpha = m_Dropped ? 0xFF : (m_Hover || (GetFocus()==this)) ? 0xF0 : 0xD0;

	CRect rectContent(rectClient);
	if (hTheme)
	{
		CRect rectBounds(rectContent);
		rectBounds.right--;
		rectBounds.bottom--;

		rectContent.DeflateRect(2, 2);
		SolidBrush brush1(Color(Alpha, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush1, rectContent.left, rectContent.top, rectContent.Width(), rectContent.Height());
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);
		Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
		g.DrawPath(&pen, &path);
		rectBounds.DeflateRect(1, 1);

		CreateRoundRectangle(rectBounds, 1, path);
		LinearGradientBrush brush2(Point(0, rectBounds.top), Point(0, rectBounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
		pen.SetBrush(&brush2);
		g.DrawPath(&pen, &path);
	}
	else
	{
		dc.Draw3dRect(rectContent, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
		rectContent.DeflateRect(1, 1);
		if (m_Dropped || (GetFocus()==this))
		{
			dc.Draw3dRect(rectContent, 0x000000, GetSysColor(COLOR_3DFACE));
			rectContent.DeflateRect(1, 1);
			dc.FillSolidRect(rectContent, GetSysColor(COLOR_WINDOW));
		}
		else
		{
			rectContent.DeflateRect(1, 1);
		}
	}

	CRect rectClip(rectContent);
	rectClip.left = rectClip.right-GetSystemMetrics(SM_CXHSCROLL);
	CRect rectArrow(rectClip);

	if (hTheme)
	{
		// Hack to achieve the same style as Windows Explorer
		if (p_App->OSVersion>OS_XP)
		{
			rectArrow.InflateRect(1, 1);
			if (m_Hover)
			{
				rectClip.left--;
			}
			else
			{
				rectArrow.InflateRect(1, 1);
			}
		}

		p_App->zDrawThemeBackground(hTheme, dc, CP_DROPDOWNBUTTON, m_Pressed ? CBXS_PRESSED : (m_Hover && !m_Dropped) ? CBXS_HOT : CBXS_NORMAL, rectArrow, rectClip);
	}
	else
	{
		if (m_Hover || m_Pressed || m_Dropped)
			dc.DrawFrameControl(rectArrow, DFC_BUTTON, DFCS_TRANSPARENT | 16 | DFCS_HOT | (m_Pressed ? DFCS_PUSHED : 0));

		dc.DrawFrameControl(rectArrow, DFC_MENU, DFCS_TRANSPARENT | 16 | (m_Pressed ? DFCS_PUSHED : (m_Hover && !m_Dropped) ? DFCS_HOT : DFCS_FLAT));
		rectClip.left--;

		if (m_Hover || m_Pressed || m_Dropped)
			dc.FillSolidRect(rectClip.left, rectClip.top, 1, rectClip.Height(), GetSysColor(COLOR_3DFACE));
	}

	CRect rectText(rectContent);
	rectText.right = rectClip.left;
	rectText.DeflateRect(BORDER, 0);

	CFont* pOldFont;

	if (m_IsEmpty)
	{
		pOldFont = dc.SelectObject(&p_App->m_ItalicFont);
		dc.SetTextColor(0x808080);
		dc.DrawText(m_EmptyHint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		pOldFont = dc.SelectObject(&p_App->m_DefaultFont);
		COLORREF c1 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000;
		COLORREF c2 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_3DSHADOW) : 0x808080;

		if (!m_Caption.IsEmpty())
		{
			dc.SetTextColor((m_Hover || m_Dropped) ? c1 : c2);
			dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
			rectText.left += dc.GetTextExtent(m_Caption).cx+BORDER;
		}

		if (m_Icon)
		{
			INT cx = GetSystemMetrics(SM_CXSMICON);
			INT cy = GetSystemMetrics(SM_CYSMICON);

			if (rectText.left+cx<rectText.right)
			{
				DrawIconEx(dc, rectText.left, rectText.top+(rectText.Height()-cy)/2, m_Icon, cx, cy, 0, NULL, DI_NORMAL);
				rectText.left += cx+BORDER;
			}
		}

		if (!m_DisplayName.IsEmpty())
		{
			dc.SetTextColor(c1);
			dc.DrawText(m_DisplayName, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
		}
	}

	dc.SelectObject(pOldFont);

	if ((GetFocus()==this) && (!m_Dropped) && (!hTheme))
	{
		rectText.InflateRect(3, -1);
		dc.SetTextColor(0x000000);
		dc.DrawFocusRect(rectText);
	}

	// Set alpha
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);
	BYTE* pBits = ((BYTE*)bmp.bmBits)+4*(rectContent.top*rectClient.Width()+rectContent.left);
	for (INT row=rectContent.top; row<rectContent.bottom; row++)
	{
		for (INT col=rectContent.left; col<rectContent.right; col++)
		{
			*(pBits+3) = Alpha;
			pBits += 4;
		}
		pBits += 4*(rectClient.Width()-rectContent.Width());
	}*/

	pDC.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBmp);
}

void CJournalButton::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
/*	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}*/
}

void CJournalButton::OnMouseLeave()
{
/*	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	Invalidate();*/
}

void CJournalButton::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
/*	SetFocus();

	if (m_Dropped)
	{
		OnCloseDropdown();
	}
	else
	{
		m_Pressed = TRUE;
		OnOpenDropdown();
	}*/
}

void CJournalButton::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
/*	if (m_Pressed)
	{
		m_Pressed = FALSE;
		Invalidate();

		ReleaseCapture();
	}*/
}
