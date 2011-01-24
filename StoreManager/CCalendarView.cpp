
// CCalendarView.cpp: Implementierung der Klasse CCalendarView
//

#include "stdafx.h"
#include "CCalendarView.h"
#include "StoreManager.h"


// CCalendarView
//

#define GetItemData(idx)     ((FVItemData*)(m_ItemData+(idx)*m_DataSize))
#define PADDING              2
#define MARGINLEFT           15-PADDING
#define GUTTER               20
#define COLUMNGUTTER         8
#define EMPTY                ((UINT)-1)

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

void CCalendarView::AdjustLayout()
{
	CRect rectWindow;
	GetWindowRect(&rectWindow);
	if (!rectWindow.Width())
		return;

	CSize sz;
	GetMonthSize(&sz);

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT col = 0;
//	INT row = 0;
	INT x = MARGINLEFT;
	INT y = MARGINLEFT;

	INT MonthsPerRow = (rectWindow.Width()-MARGINLEFT+GUTTER)/(sz.cx+GUTTER);
	MonthsPerRow = (MonthsPerRow<1) ? 1 : (MonthsPerRow==5) ? 4 : ((MonthsPerRow>6) && (MonthsPerRow<12)) ? 6 : MonthsPerRow;

	for (UINT a=0; a<12; a++)
	{
		memset(&m_Months[a].Matrix, EMPTY, sizeof(m_Months[a].Matrix));
		m_Months[a].SOM = StartOfMonth(a);
		m_Months[a].DOM = DaysOfMonth(a);

		LPRECT rect = &m_Months[a].Rect;
		rect->left = x;
		rect->top = y;
		rect->right = x+sz.cx;
		rect->bottom = y+sz.cy;

		x += sz.cx+GUTTER;
		if (rect->right>m_ScrollWidth)
			m_ScrollWidth = rect->right-1;

		if (rect->bottom>m_ScrollHeight)
		{
			m_ScrollHeight = rect->bottom;
			if ((m_ScrollHeight>rectWindow.Height()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL);
				goto Restart;
			}
		}

		if (++col>=MonthsPerRow)
		{
			col = 0;
//			row ++;
			x = MARGINLEFT;
			y += sz.cy+GUTTER;
		}
	}

	if (p_Result)
		for (UINT a=0; a<p_Result->m_ItemCount; a++)
		{
			LFItemDescriptor* i = p_Result->m_Items[a];
			if (i->AttributeValues[m_ViewParameters.SortBy])
			{
				SYSTEMTIME stUTC;
				SYSTEMTIME stLocal;
				FileTimeToSystemTime((FILETIME*)i->AttributeValues[m_ViewParameters.SortBy], &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

				if (stLocal.wYear==m_Year)
				{
					ASSERT(stLocal.wMonth<=12);
					ASSERT(stLocal.wDay<=31);
					CalendarMonth* m = &m_Months[stLocal.wMonth-1];

					m->Matrix[stLocal.wDay-1] = a;

					LPRECT rect = &GetItemData(a)->Rect;
					rect->left = m->Rect.left+CategoryPadding+((stLocal.wDay+m->SOM-1)%7)*(m_ColumnWidth+COLUMNGUTTER);
					rect->top = m->Rect.top+m_FontHeight[1]+2*CategoryPadding+((stLocal.wDay+m->SOM-1)/7)*(m_FontHeight[0]+2*PADDING-1);
					if (!m_HideDays)
						rect->top += m_FontHeight[0]+CategoryPadding;
					rect->right = rect->left+m_ColumnWidth;
					rect->bottom = rect->top+m_FontHeight[0]+2*PADDING;
				}
			}
		}

	AdjustScrollbars();
	Invalidate();
}

CMenu* CCalendarView::GetBackgroundContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_CALENDAR);
	return pMenu;
}

BOOL CCalendarView::IsLeapYear()
{
	if (m_Year%4)
		return FALSE;

	if ((m_Year%400)==0)
		return TRUE;

	return (m_Year%100)!=0;
}

UINT CCalendarView::DaysOfMonth(UINT Month)
{
	static const UINT DOM[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	ASSERT(Month<12);
	UINT Days = DOM[Month];

	if (Month==1)
		if (IsLeapYear())
			Days++;

	return Days;
}

UINT CCalendarView::StartOfMonth(UINT Month)
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

void CCalendarView::GetMonthSize(LPSIZE Size)
{
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);
	m_ColumnWidth = dc->GetTextExtent(_T("00"), 2).cx+2*PADDING;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	Size->cx = 7*m_ColumnWidth+6*COLUMNGUTTER;
	Size->cy = m_FontHeight[1]+4*CategoryPadding+6*(m_FontHeight[0]+2*PADDING-1)+1;
	if (!m_HideDays)
		Size->cy += m_FontHeight[0];
}

void CCalendarView::DrawMonth(CDC& dc, LPRECT rect, INT Month, BOOL Themed)
{
	// Header
	ItemCategory ic = { 0 };
	swprintf_s(ic.Caption, 256, m_Months[Month].Name, m_Year);
	DrawCategory(dc, rect, &ic, Themed);

	rect->top += m_FontHeight[1]+CategoryPadding;
	CRect rectItem(0, rect->top, m_ColumnWidth, rect->top+m_FontHeight[0]+2*PADDING);

	// Days
	if (!m_HideDays)
	{
		for (UINT a=0; a<7; a++)
		{
			rectItem.MoveToX(rect->left+CategoryPadding+a*(m_ColumnWidth+COLUMNGUTTER));
			dc.DrawText(m_Days[a], -1, rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_CENTER | DT_TOP);
		}

		rect->top += m_FontHeight[0]+CategoryPadding;
	}

	rect->top += CategoryPadding;

	// Matrix
	COLORREF clrDay = Themed ? 0xA8A8A8 : GetSysColor(COLOR_3DFACE);

	UINT col = m_Months[Month].SOM;
	UINT row = 0;
	for (UINT Day=0; Day<m_Months[Month].DOM; Day++)
	{
		rectItem.MoveToXY(rect->left+CategoryPadding+col*(m_ColumnWidth+COLUMNGUTTER), rect->top+row*(m_FontHeight[0]+2*PADDING-1));
		if (m_Months[Month].Matrix[Day]!=EMPTY)
		{
			DrawItemBackground(dc, rectItem, (INT)m_Months[Month].Matrix[Day], Themed);
		}
		else
		{
			dc.SetTextColor(clrDay);
		}

		CString tmpStr;
		tmpStr.Format(_T("%d"), Day+1);

		rectItem.right -= PADDING;
		dc.DrawText(tmpStr, -1, rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_RIGHT | DT_VCENTER);
		rectItem.right += PADDING;

		if (++col>=7)
		{
			col = 0;
			row++;
		}
	}
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
		wcscat_s(m_Months[a].Name, 256, L" %d");
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

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	BOOL Themed = IsCtrlThemed();
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	RECT rectIntersect;

	for (UINT a=0; a<12; a++)
	{
		CRect rect(m_Months[a].Rect);
		rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+m_HeaderHeight);
		if (IntersectRect(&rectIntersect, rect, rectUpdate))
			DrawMonth(dc, rect, a, Themed);
	}

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
