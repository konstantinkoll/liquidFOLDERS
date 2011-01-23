
// CCalendarView.h: Schnittstelle der Klasse CCalendarView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CCalendarView
//

struct CalendarMonth
{
	RECT Rect;
	WCHAR Name[256];
};

class CCalendarView : public CFileView
{
public:
	CCalendarView();

	virtual CMenu* GetBackgroundContextMenu();

protected:
	UINT m_FirstDayOfWeek;
	WCHAR m_Days[7][3];
	CalendarMonth m_Months[12];
	UINT m_Year;
	INT m_ColumnWidth;
	BOOL m_HideDays;

	virtual void SetViewOptions(BOOL Force);
	virtual void AdjustLayout();

	UINT DaysOfMonth(UINT Month);
	UINT StartOfMonth(UINT Month);
	void GetMonthSize(LPSIZE Size);
	void DrawMonth(CDC& dc, LPRECT rect, INT Month, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();

	afx_msg void OnHideDays();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	BOOL IsLeapYear();
};
