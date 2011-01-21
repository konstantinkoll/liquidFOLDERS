
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
	m_HideDays = theApp.m_HideDays;
}

CMenu* CCalendarView::GetBackgroundContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_CALENDAR);
	return pMenu;
}



BEGIN_MESSAGE_MAP(CCalendarView, CFileView)
	ON_WM_PAINT()

	ON_COMMAND(IDM_CALENDAR_HIDEDAYS, OnHideDays)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CALENDAR_HIDEDAYS, IDM_CALENDAR_GOTOYEAR, OnUpdateCommands)
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




	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}



void CCalendarView::OnHideDays()
{
	theApp.m_HideDays = !theApp.m_HideDays;
	theApp.UpdateViewOptions();
}

void CCalendarView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case IDM_CALENDAR_HIDEDAYS:
		pCmdUI->SetCheck(theApp.m_HideDays);
		break;
}

	pCmdUI->Enable(b);
}
