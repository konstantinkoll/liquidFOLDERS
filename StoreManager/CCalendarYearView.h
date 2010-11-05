
// CCalendarYearView.h: Schnittstelle der Klasse CCalendarYearView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"


// CCalendarYearView
//

class CCalendarYearView : public CFileView
{
public:
	CCalendarYearView();
	virtual ~CCalendarYearView();

	void Create(CWnd* _pParentWnd, LFSearchResult* _result, INT _FocusItem);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
