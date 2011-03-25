
// CFrameCtrl.h: Schnittstelle der Klasse CFrameCtrl
//

#pragma once
#include "liquidFOLDERS.h"


// CFrameCtrl
//

class CFrameCtrl : public CWnd
{
public:
	CFrameCtrl();

	BOOL Create(CWnd* pParentWnd, CRect rect);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	DECLARE_MESSAGE_MAP()
};
