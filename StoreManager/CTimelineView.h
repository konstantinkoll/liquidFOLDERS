
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CTimelineView
//

class CTimelineView : public CFileView
{
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
