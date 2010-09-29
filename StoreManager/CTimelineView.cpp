
// CTimelineView.cpp: Implementierung der Klasse CTimelineView
//

#include "stdafx.h"
#include "CTimelineView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CTimelineView
//

CTimelineView::CTimelineView()
{
}

CTimelineView::~CTimelineView()
{
}

void CTimelineView::Create(CWnd* _pParentWnd, LFSearchResult* _result, int _FocusItem)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewTimeline, _FocusItem);
}


BEGIN_MESSAGE_MAP(CTimelineView, CFileView)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CTimelineView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTimelineView::OnPaint()
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

	BOOL Themed = IsCtrlThemed();
	COLORREF back = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	//COLORREF text = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);

	dc.FillSolidRect(rect, back);

//	Graphics g(dc.m_hDC);
//	g.SetCompositingMode(CompositingModeSourceOver);
//
//	TODO: Paint

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
