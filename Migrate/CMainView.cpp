
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "Resource.h"
#include "Migrate.h"
#include "LFCore.h"


// CMainView
//

CMainView::CMainView()
{
	ENSURE(Hint.LoadString(IDS_EMPTYHINT));

	// Logo laden
	logo = new CGdiPlusBitmapResource();
	logo->Load(IDB_LOGO_MICRO, _T("PNG"));

	// Schrift
	fntHint.CreateFont(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		theApp.GetDefaultFontFace());

	IsRootSet = FALSE;
}

CMainView::~CMainView()
{
	if (logo)
		delete logo;
}

void CMainView::Create(CWnd* _pParentWnd)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T("Main"), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);
}

void CMainView::SetRoot(CString _Root)
{
	Root = _Root;
	IsRootSet = TRUE;
	Invalidate();
}

void CMainView::ClearRoot()
{
	Root.Empty();
	IsRootSet = FALSE;
	Invalidate();
}

BOOL CMainView::PaintEmpty(CDC* pDC, Graphics* g, CRect& rect)
{
	const int StatusHeight = 36;
	BOOL border = theApp.m_nAppLook==ID_VIEW_APPLOOK_OFF_2007_BLACK;

	COLORREF back;
	COLORREF text;
	theApp.GetBackgroundColors(border ? ChildBackground_White : ChildBackground_Ribbon, &back, &text);

	CRect r(rect);
	pDC->FillSolidRect(r, back);
	r.bottom -= StatusHeight+border-1;

	// Balken
	r.top = r.bottom;
	r.bottom = rect.bottom-border;
	SolidBrush* brush;
	if (border)
	{
		brush = new SolidBrush(Color(255, 230, 240, 250));
	}
	else
	{
		brush = new SolidBrush(Color(160, 255, 255, 255));
	}
	g->FillRectangle(brush, r.left, r.top, r.Width(), r.Height());
	delete brush;

	// Logo
	int l = logo->m_pBitmap->GetWidth();
	int h = logo->m_pBitmap->GetHeight();
	int x = rect.Width()-l-border-3;
	int y = rect.Height()-h-border-3;
	if ((x>2) && (y>2))
	{
		g->DrawImage(logo->m_pBitmap, x, y, l, h);
		x -= 2+border;
	}
	else
	{
		x = rect.right;
	}

	// Status-Text
	CFont* pOldFont = (CFont*)pDC->SelectObject(&fntHint);
	pDC->SetTextColor(0xCC3300);
	const UINT dwStyle = DT_CENTER | DT_WORD_ELLIPSIS | DT_WORDBREAK | DT_NOPREFIX;

	CRect rText(2+border, r.top, x, r.bottom);
	CRect rCalc(rText);
	pDC->DrawTextW(Hint, -1, rCalc, dwStyle | DT_CALCRECT);

	if (rCalc.Height()<=rText.Height())
	{
		rText.top = rect.bottom-StatusHeight-border+(StatusHeight-rCalc.Height())/2;
		pDC->DrawTextW(Hint, -1, rText, dwStyle);
	}

	pDC->SelectObject(pOldFont);

	return border;
}

void CMainView::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.DeflateRect(1, 1);

	int heightHdr = m_wndHeader.GetPreferredHeight();
	if (rectClient.Height()<=heightHdr)
		heightHdr = 0;

	//m_wndHeader.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightHdr, SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndTree.SetWindowPos(NULL, rectClient.left, rectClient.top+heightHdr, rectClient.Width(), rectClient.Height()-heightHdr, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CMainView::DrawBorder(CDC* pDC, CRect& rect)
{
	CPen penBorder;
	penBorder.CreatePen(PS_SOLID, 1, 0xA0A0A0);
	CPen* pOldPen = pDC->SelectObject(&penBorder);
	CBrush* pOldBrush = (CBrush*)pDC->SelectObject(GetStockObject(HOLLOW_BRUSH));
	pDC->Rectangle(rect);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_wndHeader.Create(this, 1);
	m_wndTree.Create(this, 2);

	return 0;
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMainView::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	if (IsRootSet)
	{
		DrawBorder(&pDC, rect);
	}
	else
	{
		CDC dc;
		dc.CreateCompatibleDC(&pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap buffer;
		buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&buffer);

		Graphics g(dc.m_hDC);
		g.SetCompositingMode(CompositingModeSourceOver);

		if (PaintEmpty(&dc, &g, rect))
			DrawBorder(&dc, rect);

		pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
		dc.SelectObject(pOldBitmap);
	}
}

BOOL CMainView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
	return TRUE;
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	m_wndTree.SetFocus();
}
