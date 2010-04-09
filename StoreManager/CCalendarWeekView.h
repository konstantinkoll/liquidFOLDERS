
// CCalendarWeekView.h: Schnittstelle der Klasse CCalendarWeekView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"


// CCalendarWeekView
//

class CCalendarWeekView : public CFileView
{
public:
	CCalendarWeekView();
	virtual ~CCalendarWeekView();

	void Create(CWnd* _pParentWnd, LFSearchResult* _result);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
