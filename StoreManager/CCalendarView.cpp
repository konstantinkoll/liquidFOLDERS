
// CCalendarView.cpp: Implementierung der Klasse CCalendarView
//

#include "stdafx.h"
#include "CCalendarView.h"
#include "StoreManager.h"


// CCalendarView
//

CCalendarView::CCalendarView()
	: CFileView()
{
}


BEGIN_MESSAGE_MAP(CCalendarView, CFileView)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CCalendarView::OnPaint()
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
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	CRect rectText(rect);
	rectText.top += m_HeaderHeight+6;

	dc.SetTextColor(Themed ? 0x6D6D6D : GetSysColor(COLOR_3DFACE));
	dc.DrawText(_T("Coming soon!"), -1, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}
