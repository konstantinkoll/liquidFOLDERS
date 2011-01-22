
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
	ZeroMemory(&m_Months, sizeof(m_Months));
	ZeroMemory(&m_Days, sizeof(m_Days));
}

void CCalendarView::SetViewOptions(BOOL Force)
{
	if (Force)
	{
		m_Year = 2011;	// TODO
	}

	if (Force || (m_HideDays!=theApp.m_HideDays))
	{
		m_HideDays = theApp.m_HideDays;
		AdjustLayout();
	}
}

CMenu* CCalendarView::GetBackgroundContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_CALENDAR);
	return pMenu;
}

void CCalendarView::GetMonthSize(LPSIZE Size)
{
	Size->cx = Size->cy = 150; // TODO
}


BEGIN_MESSAGE_MAP(CCalendarView, CFileView)
	ON_WM_CREATE()
	ON_WM_PAINT()

	ON_COMMAND(IDM_CALENDAR_HIDEDAYS, OnHideDays)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CALENDAR_HIDEDAYS, IDM_CALENDAR_GOTOYEAR, OnUpdateCommands)
END_MESSAGE_MAP()

INT CCalendarView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Month names
	for (UINT a=0; a<12; a++)
	{
		GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_SMONTHNAME1+a, m_Months[a].Name, 256, NULL);
		wcscat_s(m_Months[a].Name, 256, L" %d");
	}

	// Day names
	for (UINT a=0; a<7; a++)
	{
		WCHAR tmpStr[256];
		GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_SABBREVDAYNAME1+a, tmpStr, 256, NULL);
		wcsncpy_s(m_Days[a], 3, tmpStr, _TRUNCATE);
	}

	// First day of week
	WCHAR DOW[2];
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, DOW, 2);
	m_FirstDayOfWeek = DOW[0]-L'0';

	return 0;
}

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
	case IDM_CALENDAR_PREVYEAR:
		b = (m_Year>1900);
		break;
	case IDM_CALENDAR_NEXTYEAR:
		b = (m_Year<2100);
		break;
	}

	pCmdUI->Enable(b);
}
