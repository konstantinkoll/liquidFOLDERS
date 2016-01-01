
// CCalendarView.cpp: Implementierung der Klasse CCalendarView
//

#include "stdafx.h"
#include "CCalendarView.h"
#include "liquidFOLDERS.h"


// CCalendarView
//

#define GetItemData(Index)     ((CalendarItemData*)(m_ItemData+(Index)*m_DataSize))
#define PADDING                2
#define MARGINLEFT             15-PADDING
#define GUTTER                 20
#define COLUMNGUTTER           8
#define EXTRA                  (COLUMNGUTTER/2)
#define EMPTY                  ((UINT)-1)
#define MINYEAR                1900
#define MAXYEAR                2100

CCalendarView::CCalendarView()
	: CFileView(sizeof(CalendarItemData))
{
	ZeroMemory(&m_Months, sizeof(m_Months));
	ZeroMemory(&m_Days, sizeof(m_Days));
}

void CCalendarView::SetViewOptions(BOOL Force)
{
	if (Force)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		ASSERT(st.wYear>=MINYEAR);
		ASSERT(st.wYear<=MAXYEAR);
		m_Year = st.wYear;
	}

	if (Force || (m_ShowCaptions!=theApp.m_CalendarShowCaptions))
	{
		m_ShowCaptions = theApp.m_CalendarShowCaptions;
		AdjustLayout();
	}
}

void CCalendarView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	CFileView::SetSearchResult(pRawFiles, pCookedFiles, Data);

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			LFItemDescriptor* i = p_CookedFiles->m_Items[a];
			if (i->AttributeValues[m_ViewParameters.SortBy])
				if (*((INT64*)i->AttributeValues[m_ViewParameters.SortBy]))
				{
					CalendarItemData* d = GetItemData(a);
					d->Hdr.Valid = TRUE;

					SYSTEMTIME stUTC;
					FileTimeToSystemTime((FILETIME*)i->AttributeValues[m_ViewParameters.SortBy], &stUTC);
					SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &d->Time);
				}
		}

	if (Data)
		if ((Data->Year>=MINYEAR) && (Data->Year<=MAXYEAR))
			m_Year = Data->Year;
}

void CCalendarView::SetYear(UINT Year)
{
	ASSERT(Year>=MINYEAR);
	ASSERT(Year<=MAXYEAR);
	m_Year = Year;

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			if (GetItemData(a)->Time.wYear==Year)
			{
				m_FocusItem = a;
				break;
			}

	AdjustLayout();
}

void CCalendarView::AdjustLayout()
{
	CRect rectWindow;
	GetWindowRect(rectWindow);
	if (!rectWindow.Width())
		return;

	CSize sz;
	GetMonthSize(&sz);

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT x = MARGINLEFT;
	INT y = MARGINLEFT;

	INT MonthsPerRow = (rectWindow.Width()-MARGINLEFT+GUTTER)/(sz.cx+GUTTER);
	MonthsPerRow = (MonthsPerRow<1) ? 1 : (MonthsPerRow==5) ? 4 : ((MonthsPerRow>6) && (MonthsPerRow<12)) ? 6 : MonthsPerRow;

	for (UINT a=0; a<12; a++)
	{
		memset(&m_Months[a].Matrix, EMPTY, sizeof(m_Months[a].Matrix));
		m_Months[a].SOM = StartOfMonth(a);
		m_Months[a].DOM = DaysOfMonth(a);

		const LPRECT lpRect = &m_Months[a].Rect;

		lpRect->left = x;
		lpRect->top = y;
		lpRect->right = x+sz.cx;
		lpRect->bottom = y+sz.cy;

		x += sz.cx+GUTTER;
		if (lpRect->right>m_ScrollWidth)
			m_ScrollWidth = lpRect->right-1;

		if (lpRect->bottom>m_ScrollHeight)
		{
			m_ScrollHeight = lpRect->bottom;
			if ((m_ScrollHeight>rectWindow.Height()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL);
				goto Restart;
			}
		}

		if (++Column>=MonthsPerRow)
		{
			Column = 0;
			x = MARGINLEFT;
			y += sz.cy+GUTTER;
		}
	}

	if (p_CookedFiles)
	{
		const INT FontHeight = theApp.m_DefaultFont.GetFontHeight();

		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			CalendarItemData* d = GetItemData(a);
			if (d->Time.wYear==m_Year)
			{
				ASSERT(d->Time.wMonth<=12);
				ASSERT(d->Time.wDay<=31);
				CalendarMonth* m = &m_Months[d->Time.wMonth-1];

				UINT Day = d->Time.wDay-1;
				m->Matrix[Day] = a;

				Day += m->SOM;

				const LPRECT lpRect = &d->Hdr.Rect;

				lpRect->left = m->Rect.left+LFCategoryPadding+(Day%7)*(m_ColumnWidth+COLUMNGUTTER);
				lpRect->top = m->Rect.top+theApp.m_LargeFont.GetFontHeight()+2*LFCategoryPadding+(Day/7)*(FontHeight+2*PADDING-1);

				if (m_ShowCaptions)
					lpRect->top += FontHeight+LFCategoryPadding;

				lpRect->right = lpRect->left+m_ColumnWidth;
				lpRect->bottom = lpRect->top+FontHeight+2*PADDING;
			}
		}
	}

	CFileView::AdjustLayout();
}

CMenu* CCalendarView::GetViewContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_CALENDAR);

	return pMenu;
}

void CCalendarView::GetPersistentData(FVPersistentData& Data) const
{
	CFileView::GetPersistentData(Data);

	Data.Year = m_Year;
}

BOOL CCalendarView::IsLeapYear() const
{
	if (m_Year&3)
		return FALSE;

	if ((m_Year%400)==0)
		return TRUE;

	return (m_Year%100)!=0;
}

UINT CCalendarView::DaysOfMonth(UINT Month) const
{
	static const UINT DOM[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	ASSERT(Month<12);
	UINT Days = DOM[Month];

	if (Month==1)
		if (IsLeapYear())
			Days++;

	return Days;
}

UINT CCalendarView::StartOfMonth(UINT Month) const
{
	static const UINT SOM[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };

	ASSERT(Month<12);
	UINT D = (7-m_FirstDayOfWeek)%7;
	UINT M = SOM[Month];
	UINT Y = ((m_Year%100)+((m_Year%100)/4))%7;
	UINT C = (3-((m_Year/100)%4))*2;

	if (Month<=1)
		if (IsLeapYear())
			D += 6;

	return (D+M+Y+C)%7;
}

void CCalendarView::GetMonthSize(LPSIZE lpSize)
{
	ASSERT(lpSize);

	m_ColumnWidth = theApp.m_DefaultFont.GetTextExtent(_T("00")).cx+2*PADDING;

	lpSize->cx = 7*m_ColumnWidth+6*COLUMNGUTTER;
	lpSize->cy = theApp.m_LargeFont.GetFontHeight()+4*LFCategoryPadding+6*(theApp.m_DefaultFont.GetFontHeight()+2*PADDING-1)+1;

	if (m_ShowCaptions)
		lpSize->cy += theApp.m_DefaultFont.GetFontHeight();
}

void CCalendarView::DrawMonth(CDC& dc, LPRECT lpRect, INT Month, BOOL Themed)
{
	ASSERT(GUTTER>COLUMNGUTTER);

	const INT FontHeight = theApp.m_DefaultFont.GetFontHeight();

	// Header
	ItemCategory ic = { 0 };
	swprintf_s(ic.Caption, 256, m_Months[Month].Name, m_Year);
	DrawCategory(dc, lpRect, ic.Caption, ic.Hint, Themed);

	lpRect->top += theApp.m_LargeFont.GetFontHeight()+LFCategoryPadding;
	CRect rectItem(0, lpRect->top, m_ColumnWidth+2*EXTRA, lpRect->top+FontHeight+2*PADDING);

	// Days
	if (m_ShowCaptions)
	{
		for (UINT a=0; a<7; a++)
		{
			rectItem.MoveToX(lpRect->left+LFCategoryPadding+a*(m_ColumnWidth+COLUMNGUTTER)-EXTRA);
			dc.DrawText(m_Days[a], rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_CENTER | DT_TOP);
		}

		lpRect->top += FontHeight+LFCategoryPadding;
	}

	lpRect->top += LFCategoryPadding;
	rectItem.DeflateRect(EXTRA, 0);

	// Matrix
	COLORREF clrDay = Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW);

	UINT Column = m_Months[Month].SOM;
	UINT Row = 0;

	for (UINT Day=0; Day<m_Months[Month].DOM; Day++)
	{
		rectItem.MoveToXY(lpRect->left+LFCategoryPadding+Column*(m_ColumnWidth+COLUMNGUTTER), lpRect->top+Row*(FontHeight+2*PADDING-1));

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

		if (m_Months[Month].Matrix[Day]!=EMPTY)
			DrawItemForeground(dc, rectItem, (INT)m_Months[Month].Matrix[Day], Themed);

		rectItem.right -= PADDING;
		dc.DrawText(tmpStr, rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_RIGHT | DT_VCENTER);
		rectItem.right += PADDING;

		if (++Column>=7)
		{
			Column = 0;
			Row++;
		}
	}
}


BEGIN_MESSAGE_MAP(CCalendarView, CFileView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()

	ON_COMMAND(IDM_CALENDAR_SHOWCAPTIONS, OnShowCaptions)
	ON_COMMAND(IDM_CALENDAR_PREVYEAR, OnPrevYear)
	ON_COMMAND(IDM_CALENDAR_NEXTYEAR, OnNextYear)
	ON_COMMAND(IDM_CALENDAR_GOTOYEAR, OnGoToYear)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CALENDAR_SHOWCAPTIONS, IDM_CALENDAR_GOTOYEAR, OnUpdateCommands)
END_MESSAGE_MAP()

INT CCalendarView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

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

void CCalendarView::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	RECT rectIntersect;

	for (UINT a=0; a<12; a++)
	{
		CRect rect(m_Months[a].Rect);
		rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

		if (IntersectRect(&rectIntersect, rect, rectUpdate))
			DrawMonth(dc, rect, a, Themed);
	}

	DrawWindowEdge(dc, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
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
	theApp.m_CalendarShowCaptions = !theApp.m_CalendarShowCaptions;
	theApp.UpdateViewOptions();
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
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_CALENDAR_SHOWCAPTIONS:
		pCmdUI->SetCheck(theApp.m_CalendarShowCaptions);
		break;

	case IDM_CALENDAR_PREVYEAR:
		b = (m_Year>MINYEAR);
		break;

	case IDM_CALENDAR_NEXTYEAR:
		b = (m_Year<MAXYEAR);
		break;
	}

	pCmdUI->Enable(b);
}
