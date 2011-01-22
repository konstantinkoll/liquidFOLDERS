
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
	UINT m_Year;
	CalendarMonth m_Months[12];
	WCHAR m_Days[7][3];
	UINT m_FirstDayOfWeek;
	BOOL m_HideDays;

	virtual void SetViewOptions(BOOL Force);

	void GetMonthSize(LPSIZE Size);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();

	afx_msg void OnHideDays();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
