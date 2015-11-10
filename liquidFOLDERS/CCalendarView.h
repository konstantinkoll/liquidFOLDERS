
// CCalendarView.h: Schnittstelle der Klasse CCalendarView
//

#pragma once
#include "CFileView.h"


// CCalendarView
//

struct CalendarItemData
{
	FVItemData Hdr;
	SYSTEMTIME Time;
};

struct CalendarMonth
{
	RECT Rect;
	WCHAR Name[256];
	UINT Matrix[31];
	UINT SOM;			// Start of month
	UINT DOM;			// Days of month
};

class CCalendarView : public CFileView
{
public:
	CCalendarView();

	virtual CMenu* GetViewContextMenu();
	virtual void GetPersistentData(FVPersistentData& Data) const;

protected:
	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual void AdjustLayout();

	void SetYear(UINT Year);
	UINT DaysOfMonth(UINT Month) const;
	UINT StartOfMonth(UINT Month) const;
	void GetMonthSize(LPSIZE lpSize);
	void DrawMonth(CDC& dc, LPRECT lpRect, INT Month, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnShowCaptions();
	afx_msg void OnPrevYear();
	afx_msg void OnNextYear();
	afx_msg void OnGoToYear();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	UINT m_FirstDayOfWeek;
	WCHAR m_Days[7][3];
	CalendarMonth m_Months[12];
	UINT m_Year;
	INT m_ColumnWidth;
	BOOL m_ShowCaptions;

private:
	BOOL IsLeapYear() const;
};
