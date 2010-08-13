
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

UINT CDropdownSelector::GetPreferredHeight()
{
	LOGFONT lf;
	((LFApplication*)AfxGetApp())->m_DefaultFont.GetLogFont(&lf);
	UINT h = max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON));

	return h+2*BORDER;
}


BEGIN_MESSAGE_MAP(CDropdownSelector, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

int CDropdownSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	return 0;
}

void CDropdownSelector::OnDestroy()
{
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

	CRect rtext(rcontent);
	rtext.DeflateRect(BORDER, 0);

	CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_ItalicFont);
	dc.SetTextColor(0x808080);
	dc.DrawText(m_EmptyHint, -1, rtext, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
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
