
// CGlasEdit.cpp: Implementierung der Klasse CGlasEdit
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CGlasEdit
//

#define BORDER     4

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CGlasEdit::CGlasEdit()
	: CEdit()
{
	hSearchIcon = NULL;
	m_IconSize = m_ClientAreaTopOffset = m_FontHeight = 0;
	m_Hover = FALSE;
}

BOOL CGlasEdit::Create(CString EmptyHint, CGlasWindow* pParentWnd, UINT nID, BOOL ShowSearchIcon)
{
	m_EmptyHint = EmptyHint;
	m_ShowSearchIcon = ShowSearchIcon;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(0, _T("EDIT"), _T(""), dwStyle, rect, pParentWnd, nID);
}

UINT CGlasEdit::GetPreferredHeight()
{
	return max(m_FontHeight, GetSystemMetrics(SM_CYSMICON))+2*BORDER;
}


BEGIN_MESSAGE_MAP(CGlasEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()

INT CGlasEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct)==-1)
		return -1;

	LFApplication* pApp = (LFApplication*)AfxGetApp();
	SetFont(&pApp->m_DefaultFont);

	if (m_ShowSearchIcon)
	{
		LOGFONT lf;
		pApp->m_DefaultFont.GetLogFont(&lf);
		UINT h = max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON));

		if (h>=16)
		{
			m_IconSize = (h>=27) ? 27 : (h>=22) ? 22 : 16;
			hSearchIcon = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDI_SEARCH), IMAGE_ICON, m_IconSize, m_IconSize, LR_DEFAULTCOLOR);
		}
	}

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_FontHeight = dc->GetTextExtent(_T("Wy")).cy;
	ReleaseDC(dc);

	m_ClientAreaTopOffset = max(0, (GetPreferredHeight()-BORDER-m_FontHeight)/2-2);
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);

	return 0;
}

void CGlasEdit::OnDestroy()
{
	CWnd::OnDestroy();

	if (hSearchIcon)
		DestroyIcon(hSearchIcon);
}

BOOL CGlasEdit::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGlasEdit::OnNcPaint()
{
	CRect rect;
	GetWindowRect(&rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.OffsetRect(BORDER+3, BORDER+m_ClientAreaTopOffset);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rect.Width();
	dib.bmiHeader.biHeight = -rect.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmp);

	CGlasWindow* pCtrlSite = (CGlasWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rect);
	const BYTE Alpha = (m_Hover || (GetFocus()==this)) ? (pCtrlSite->GetDesign()==GWD_THEMED) ? 0xFF : 0xF0 : 0xD0;

	Graphics g(dc);
	g.ExcludeClip(Rect(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height()));

	CRect rectContent(rect);
	if (IsCtrlThemed())
	{
		CRect rectBounds(rect);
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
		if (GetFocus()==this)
		{
			dc.Draw3dRect(rectContent, 0x000000, GetSysColor(COLOR_3DFACE));
			rectContent.DeflateRect(1, 1);
			dc.FillSolidRect(rectContent, GetSysColor(COLOR_WINDOW));
		}
	}

	CDC dcPaint;
	dcPaint.CreateCompatibleDC(&pDC);
	dcPaint.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&dc, rectClient.Width(), rectClient.Height());
	CBitmap* pOldBitmap = dcPaint.SelectObject(&buffer);

	CEdit::DefWindowProc(WM_PAINT, (WPARAM)dcPaint.m_hDC, NULL);

	if (pCtrlSite->GetDesign()>=GWD_THEMED)
	{
		BLENDFUNCTION BF = { AC_SRC_OVER, 0, Alpha, 0 };
		AlphaBlend(dc, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), dcPaint, 0, 0, rectClient.Width(), rectClient.Height(), BF);
	}
	else
	{
		dc.BitBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), &dcPaint, 0, 0, SRCCOPY);
	}

	dcPaint.SelectObject(pOldBitmap);

	if ((GetWindowTextLength()==0) && (GetFocus()!=this))
	{
		CRect rectText(rectContent);
		rectText.DeflateRect(BORDER, 0);
		if (m_IconSize)
		{
			DrawIconEx(dc, rectText.right-m_IconSize, rectText.top+(rectText.Height()-m_IconSize)/2, hSearchIcon, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);
			rectText.right -= m_IconSize+BORDER;
		}

		CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_ItalicFont);
		dc.SetTextColor(0x808080);
		dc.DrawText(m_EmptyHint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
		dc.SelectObject(pOldFont);
	}

	// Set alpha
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);
	BYTE* pBits = ((BYTE*)bmp.bmBits)+4*(rectContent.top*rect.Width()+rectContent.left);
	for (INT row=rectContent.top; row<rectContent.bottom; row++)
	{
		for (INT col=rectContent.left; col<rectContent.right; col++)
		{
			*(pBits+3) = Alpha;
			pBits += 4;
		}
		pBits += 4*(rect.Width()-rectContent.Width());
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBmp);
}

void CGlasEdit::OnPaint()
{
	CPaintDC pDC(this);
	OnNcPaint();
}

void CGlasEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}

	CEdit::OnMouseMove(nFlags, point);
}

void CGlasEdit::OnMouseLeave()
{
	m_Hover = FALSE;
	Invalidate();
}

void CGlasEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);

	lpncsp->rgrc[0].top += BORDER+m_ClientAreaTopOffset;
	lpncsp->rgrc[0].bottom -= BORDER;
	lpncsp->rgrc[0].left += 3+BORDER;
	lpncsp->rgrc[0].right -= BORDER;
}
