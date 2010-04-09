
// CCalendarHeaderCtrl.cpp: Implementierung der Klasse CCalendarHeaderCtrl
//

#include "stdafx.h"
#include "CCalendarHeaderCtrl.h"
#include "StoreManager.h"


// CCalendarHeaderCtrl
//

CCalendarHeaderCtrl::CCalendarHeaderCtrl()
	: CWnd()
{
	m_TextCol = 0x000000;
	m_BackCol = m_LineCol = 0xFFFFFF;
}

CCalendarHeaderCtrl::~CCalendarHeaderCtrl()
{
}

BOOL CCalendarHeaderCtrl::Create(CFileView* pParentWnd, UINT nID)
{

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	return CWnd::Create(className, _T("Header"), dwStyle, rect, pParentWnd, nID);
}

void CCalendarHeaderCtrl::SetText(CString _Text)
{
	m_Text = _Text;
	Invalidate();
}

void CCalendarHeaderCtrl::SetColors(COLORREF _TextCol, COLORREF _BackCol, COLORREF _LineCol)
{
	m_TextCol = _TextCol;
	m_BackCol = _BackCol;
	m_LineCol = _LineCol;
	Invalidate();
}


BEGIN_MESSAGE_MAP(CCalendarHeaderCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CCalendarHeaderCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CCalendarHeaderCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);
	
	CRect r = rect;
	r.top = r.bottom-1;
	dc.FillSolidRect(r, m_LineCol);

	r = rect;
	r.bottom--;
	dc.FillSolidRect(r, m_BackCol);
	
	CFont* pOldFont = (CFont*)dc.SelectObject(&theApp.m_Fonts[TRUE][TRUE]);
	dc.SetTextColor(m_TextCol);

	dc.DrawText(m_Text, r, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_CENTER);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}


void CCalendarHeaderCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}
