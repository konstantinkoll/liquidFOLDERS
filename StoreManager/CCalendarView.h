
// CCalendarView.h: Schnittstelle der Klasse CCalendarView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CCalendarView
//

class CCalendarView : public CFileView
{
public:
	CCalendarView();

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
