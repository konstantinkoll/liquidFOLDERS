
// CCalendarView.cpp: Implementierung der Klasse CCalendarView
//

#include "stdafx.h"
#include "CCalendarView.h"
#include "liquidFOLDERS.h"


// CCalendarView
//

#define ITEMPADDING      2
#define MARGIN           BACKSTAGEBORDER
#define COLUMNGUTTER     8
#define EXTRA            (COLUMNGUTTER/2)
#define EMPTY            ((UINT)-1)
#define MINYEAR          1900
#define MAXYEAR          2100

CCalendarView::CCalendarView()
	: CFileView(FRONTSTAGE_CARDBACKGROUND | FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLESELECTION | FF_ENABLEFOLDERTOOLTIPS | FF_ENABLETOOLTIPICONS, sizeof(CalendarItemData))
{
	ZeroMemory(&m_Months, sizeof(m_Months));
	ZeroMemory(&m_Days, sizeof(m_Days));
}

void CCalendarView::SetViewSettings(BOOL UpdateSearchResultPending)
{
	if (m_GlobalViewSettings.CalendarShowDays!=theApp.m_GlobalViewSettings.CalendarShowDays)
		m_GlobalViewSettings.CalendarShowDays = theApp.m_GlobalViewSettings.CalendarShowDays;

	// Commit settings
	if (!UpdateSearchResultPending)
		AdjustLayout();
}

void CCalendarView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	// Calendar is always visible!
	m_Nothing = FALSE;

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];

			if (pItemDescriptor->AttributeValues[m_ContextViewSettings.SortBy])
				if (*((INT64*)pItemDescriptor->AttributeValues[m_ContextViewSettings.SortBy]))
				{
					CalendarItemData* pData = GetCalendarItemData(a);

					SYSTEMTIME stUTC;
					FileTimeToSystemTime((FILETIME*)pItemDescriptor->AttributeValues[m_ContextViewSettings.SortBy], &stUTC);
					SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &pData->Time);
				}
		}

	if (pPersistentData)
		if ((pPersistentData->Year>=MINYEAR) && (pPersistentData->Year<=MAXYEAR))
			m_Year = pPersistentData->Year;
}

void CCalendarView::SetYear(UINT Year)
{
	ASSERT((Year>=MINYEAR) && (Year<=MAXYEAR));

	m_Year = Year;

	if (p_CookedFiles)
		for (UINT Index=0; Index<p_CookedFiles->m_ItemCount; Index++)
			if (GetCalendarItemData(Index)->Time.wYear==Year)
			{
				m_FocusItem = Index;

				break;
			}

	AdjustLayout();
}

BOOL CCalendarView::GetContextMenu(CMenu& Menu, INT Index)
{
	if (Index==-1)
		Menu.LoadMenu(IDM_CALENDAR);

	return CFileView::GetContextMenu(Menu, Index);
}

void CCalendarView::GetPersistentData(FVPersistentData& Data, BOOL ForReload) const
{
	CFileView::GetPersistentData(Data, ForReload);

	Data.Year = m_Year;
}

void CCalendarView::AdjustLayout()
{
	CRect rectLayout;
	GetWindowRect(rectLayout);

	if (!rectLayout.Width())
		return;

	CSize sz;
	GetMonthSize(&sz);

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = MARGIN;
	INT y = MARGIN;

	INT MonthsPerRow = (rectLayout.Width()-MARGIN)/(sz.cx+MARGIN);

	// Only allow some layouts
	MonthsPerRow = (MonthsPerRow<1) ? 1 : (MonthsPerRow==5) ? 4 : ((MonthsPerRow>6) && (MonthsPerRow<12)) ? 6 : MonthsPerRow;

	// Layout months
	for (UINT a=0; a<12; a++)
	{
		memset(&m_Months[a].Matrix, EMPTY, sizeof(m_Months[a].Matrix));
		m_Months[a].Column = Column;
		m_Months[a].Row = Row;
		m_Months[a].SOM = StartOfMonth(a);
		m_Months[a].DOM = DaysOfMonth(a);

		const LPRECT lpRect = &m_Months[a].Rect;
		lpRect->right = (lpRect->left=x)+sz.cx;
		lpRect->bottom = (lpRect->top=y)+sz.cy;

		x += sz.cx+MARGIN;
		if (lpRect->right>m_ScrollWidth)
			m_ScrollWidth = lpRect->right-1;

		if (lpRect->bottom+MARGIN>m_ScrollHeight)
			if (((m_ScrollHeight=lpRect->bottom+MARGIN)>rectLayout.Height()) && !HasScrollbars)
			{
				HasScrollbars = TRUE;
				rectLayout.right -= GetSystemMetrics(SM_CXVSCROLL);

				goto Restart;
			}

		if (++Column>=MonthsPerRow)
		{
			Column = 0;
			Row++;
			x = MARGIN;
			y += sz.cy+MARGIN;
		}
	}

	// Layout days/items
	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			CalendarItemData* pData = GetCalendarItemData(a);

			if ((pData->Hdr.Valid=(pData->Time.wYear==m_Year))==TRUE)
			{
				ASSERT(pData->Time.wMonth<=12);
				ASSERT(pData->Time.wDay<=31);

				CalendarMonth* pMonth = &m_Months[pData->Time.wMonth-1];

				UINT Day = pData->Time.wDay-1;
				pMonth->Matrix[Day] = a;

				Day += pMonth->SOM;

				pData->Hdr.Rect.left = pMonth->Rect.left+(Day%7)*(m_ColumnWidth+COLUMNGUTTER)+CARDPADDING;
				pData->Hdr.Rect.top = pMonth->Rect.top+m_LargeFontHeight+2*LFCATEGORYPADDING+(Day/7)*(m_DefaultFontHeight+2*ITEMPADDING-1)+CARDPADDING;

				if (m_GlobalViewSettings.CalendarShowDays)
					pData->Hdr.Rect.top += m_DefaultFontHeight+LFCATEGORYPADDING;

				pData->Hdr.Rect.right = pData->Hdr.Rect.left+m_ColumnWidth;
				pData->Hdr.Rect.bottom = pData->Hdr.Rect.top+m_DefaultFontHeight+2*ITEMPADDING;

				pData->Hdr.Column = pMonth->Column*7+(Day%7);
				pData->Hdr.Row = pMonth->Row*6+(Day/7);
			}
	}

	CFileView::AdjustLayout();
}

BOOL CCalendarView::IsLeapYear() const
{
	if (m_Year & 3)
		return FALSE;

	if ((m_Year % 400)==0)
		return TRUE;

	return (m_Year % 100)!=0;
}

UINT CCalendarView::DaysOfMonth(UINT Month) const
{
	ASSERT(Month<12);

	static const UINT DOM[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	UINT Days = DOM[Month];

	if (Month==1)
		if (IsLeapYear())
			Days++;

	return Days;
}

UINT CCalendarView::StartOfMonth(UINT Month) const
{
	ASSERT(Month<12);

	static const UINT SOM[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };

	UINT D = (7-m_FirstDayOfWeek) % 7;
	UINT M = SOM[Month];
	UINT Y = ((m_Year % 100)+((m_Year % 100)/4)) % 7;
	UINT C = (3-((m_Year/100) % 4))*2;

	if (Month<=1)
		if (IsLeapYear())
			D += 6;

	return (D+M+Y+C) % 7;
}

void CCalendarView::GetMonthSize(LPSIZE lpSize)
{
	ASSERT(lpSize);

	m_ColumnWidth = theApp.m_DefaultFont.GetTextExtent(_T("00")).cx+2*ITEMPADDING;

	lpSize->cx = 7*m_ColumnWidth+6*COLUMNGUTTER+2*CARDPADDING;
	lpSize->cy = m_LargeFontHeight+2*LFCATEGORYPADDING+6*(m_DefaultFontHeight+2*ITEMPADDING-1)+1+2*CARDPADDING;

	if (m_GlobalViewSettings.CalendarShowDays)
		lpSize->cy += m_DefaultFontHeight+LFCATEGORYPADDING;
}

void CCalendarView::DrawMonth(CDC& dc, Graphics& g, CRect& rectMonth, INT Month, BOOL Themed)
{
	// Card
	DrawCardForeground(dc, g, rectMonth, Themed);

	rectMonth.DeflateRect(CARDPADDING, CARDPADDING);
	rectMonth.top -= LFCATEGORYPADDING;

	// Header
	WCHAR tmpStr[256];
	swprintf_s(tmpStr, 256, m_Months[Month].Name, m_Year);

	DrawCategory(dc, rectMonth, tmpStr, NULL, Themed);

	rectMonth.top += m_LargeFontHeight+2*LFCATEGORYPADDING;
	CRect rectItem(0, rectMonth.top, m_ColumnWidth+2*EXTRA, rectMonth.top+m_DefaultFontHeight+2*ITEMPADDING);

	// Days
	if (m_GlobalViewSettings.CalendarShowDays)
	{
		for (UINT a=0; a<7; a++)
		{
			rectItem.MoveToX(rectMonth.left+a*(m_ColumnWidth+COLUMNGUTTER)-EXTRA);
			dc.DrawText(m_Days[a], rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_CENTER | DT_TOP);
		}

		rectMonth.top += m_DefaultFontHeight+LFCATEGORYPADDING;
	}

	rectMonth.top += LFCATEGORYPADDING;
	rectItem.DeflateRect(EXTRA, 0);

	// Matrix
	COLORREF clrDay = Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW);

	UINT Column = m_Months[Month].SOM;
	UINT Row = 0;

	for (UINT Day=0; Day<m_Months[Month].DOM; Day++)
	{
		rectItem.MoveToXY(rectMonth.left+Column*(m_ColumnWidth+COLUMNGUTTER), rectMonth.top+Row*(m_DefaultFontHeight+2*ITEMPADDING-1));

		if (m_Months[Month].Matrix[Day]!=EMPTY)
		{
			DrawItemBackground(dc, rectItem, (INT)m_Months[Month].Matrix[Day], Themed);
		}
		else
		{
			dc.SetTextColor(clrDay);
		}

		CString tmpStr;
		tmpStr.Format(_T("%u"), Day+1);

		rectItem.right -= ITEMPADDING;
		dc.DrawText(tmpStr, rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_RIGHT | DT_VCENTER);
		rectItem.right += ITEMPADDING;

		if (m_Months[Month].Matrix[Day]!=EMPTY)
			DrawItemForeground(dc, rectItem, (INT)m_Months[Month].Matrix[Day], Themed);

		if (++Column>=7)
		{
			Column = 0;
			Row++;
		}
	}
}

void CCalendarView::DrawStage(CDC& dc, Graphics& g, const CRect& /*rect*/, const CRect& rectUpdate, BOOL Themed)
{
	RECT rectIntersect;

	for (UINT a=0; a<12; a++)
	{
		CRect rect(m_Months[a].Rect);
		rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

		if (IntersectRect(&rectIntersect, rect, rectUpdate))
			DrawMonth(dc, g, rect, a, Themed);
	}
}


BEGIN_MESSAGE_MAP(CCalendarView, CFileView)
	ON_WM_CREATE()
	ON_WM_KEYDOWN()

	ON_COMMAND(IDM_CALENDAR_SHOWDAYS, OnShowCaptions)
	ON_COMMAND(IDM_CALENDAR_PREVYEAR, OnPrevYear)
	ON_COMMAND(IDM_CALENDAR_NEXTYEAR, OnNextYear)
	ON_COMMAND(IDM_CALENDAR_GOTOYEAR, OnGoToYear)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CALENDAR_SHOWDAYS, IDM_CALENDAR_GOTOYEAR, OnUpdateCommands)
END_MESSAGE_MAP()

INT CCalendarView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Current year
	SYSTEMTIME st;
	GetLocalTime(&st);

	ASSERT((st.wYear>=MINYEAR) && (st.wYear<=MAXYEAR));
	m_Year = st.wYear;

	// First day of week
	WCHAR DOW[2];
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, DOW, 2);
	m_FirstDayOfWeek = DOW[0]-L'0';

	// Day names
	for (UINT a=0; a<7; a++)
	{
		WCHAR tmpStr[256];
		GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_SABBREVDAYNAME1+(a+m_FirstDayOfWeek)%7, tmpStr, 256, NULL);
		wcsncpy_s(m_Days[a], 3, tmpStr, _TRUNCATE);
	}

	// Month names
	for (UINT a=0; a<12; a++)
	{
		GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_SMONTHNAME1+a, m_Months[a].Name, 256, NULL);
		wcscat_s(m_Months[a].Name, 256, L" %u");
	}

	return 0;
}

void CCalendarView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	if (nChar=='Y')
		if ((GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0))
			OnGoToYear();
}


void CCalendarView::OnShowCaptions()
{
	p_GlobalViewSettings->CalendarShowDays = !p_GlobalViewSettings->CalendarShowDays;

	theApp.UpdateViewSettings(-1, LFViewCalendar);
}

void CCalendarView::OnPrevYear()
{
	SetYear(m_Year-1);
}

void CCalendarView::OnNextYear()
{
	SetYear(m_Year+1);
}

void CCalendarView::OnGoToYear()
{
	LFGotoYearDlg dlg(m_Year, this);
	if (dlg.DoModal()==IDOK)
		SetYear(dlg.m_Year);
}

void CCalendarView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_CALENDAR_SHOWDAYS:
		pCmdUI->SetCheck(m_GlobalViewSettings.CalendarShowDays);
		break;

	case IDM_CALENDAR_PREVYEAR:
		bEnable = (m_Year>MINYEAR);
		break;

	case IDM_CALENDAR_NEXTYEAR:
		bEnable = (m_Year<MAXYEAR);
		break;
	}

	pCmdUI->Enable(bEnable);
}
