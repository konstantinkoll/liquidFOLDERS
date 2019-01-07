
// CCalendarView.h: Schnittstelle der Klasse CCalendarView
//

#pragma once
#include "CFileView.h"


// CCalendarView
//

struct CalendarItemData
{
	ItemData Hdr;
	SYSTEMTIME Time;
};

struct CalendarMonth
{
	RECT Rect;
	INT Column;
	INT Row;
	WCHAR Name[256];
	UINT Matrix[31];
	UINT SOM;			// Start of month
	UINT DOM;			// Days of month
};

class CCalendarView sealed : public CFileView
{
public:
	CCalendarView();

	virtual void GetPersistentData(FVPersistentData& Data, BOOL ForReload=FALSE) const;
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

protected:
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);

	void GetMonthSize(LPSIZE lpSize);
	UINT DaysOfMonth(UINT Month) const;
	UINT StartOfMonth(UINT Month) const;
	void DrawMonth(CDC& dc, Graphics& g, CRect& rectMonth, INT Month, BOOL Themed);
	void SetYear(UINT Year);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
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
	CalendarItemData* GetCalendarItemData(INT Index) const;
	BOOL IsLeapYear() const;
};

inline CalendarItemData* CCalendarView::GetCalendarItemData(INT Index) const
{
	return (CalendarItemData*)GetItemData(Index);
}
