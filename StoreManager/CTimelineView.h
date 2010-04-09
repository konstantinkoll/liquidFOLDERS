
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "liquidFOLDERS.h"
#include "StoreManager.h"
#include "CFileView.h"


// CTimelineView
//

class CTimelineView : public CFileView
{
public:
	CTimelineView();
	virtual ~CTimelineView();

	void Create(CWnd* _pParentWnd, LFSearchResult* _result);

protected:

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
