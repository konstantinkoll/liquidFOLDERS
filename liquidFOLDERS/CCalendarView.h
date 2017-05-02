
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
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void SetYear(UINT Year);
	UINT DaysOfMonth(UINT Month) const;
	UINT StartOfMonth(UINT Month) const;
	void GetMonthSize(LPSIZE lpSize);
	void DrawMonth(CDC& dc, Graphics& g, CRect& rectMonth, INT Month, BOOL Themed);

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

private:
	BOOL IsLeapYear() const;
};
