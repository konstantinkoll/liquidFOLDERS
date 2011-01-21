
// CCalendarView.h: Schnittstelle der Klasse CCalendarView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CCalendarView
//

struct CalendarMonth
{
	RECT rect;

};

class CCalendarView : public CFileView
{
public:
	CCalendarView();

	virtual CMenu* GetBackgroundContextMenu();

protected:
	UINT m_Year;
	CalendarMonth m_Months[12];
	BOOL m_HideDays;

	afx_msg void OnPaint();

	afx_msg void OnHideDays();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
