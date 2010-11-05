
// CCalendarDayView.h: Schnittstelle der Klasse CCalendarDayView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CListView.h"
#include "CFileList.h"
#include "CCalendarHeaderCtrl.h"


// CCalendarDayView
//

class CCalendarDayView : public CAbstractListView
{
public:
	CCalendarDayView();
	virtual ~CCalendarDayView();

	void Create(CWnd* _pParentWnd, LFSearchResult* _result, INT _FocusItem);

protected:
	CCalendarHeaderCtrl m_CalendarHeaderCtrl;

	virtual void SetViewOptions(UINT _ViewID, BOOL Force);
	virtual void SetSearchResult(LFSearchResult* _result);

	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()
};
