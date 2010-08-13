
// CDropdownSelector.h: Schnittstelle der Klasse CDropdownSelector
//

#pragma once
#include "CGlasWindow.h"


// CDropdownSelector
//

class AFX_EXT_CLASS CDropdownSelector : public CWnd
{
public:
	CDropdownSelector();

	BOOL Create(CGlasWindow* pParentWnd, UINT nID);
	UINT GetPreferredHeight();

protected:
	BOOL m_Hover;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	DECLARE_MESSAGE_MAP()
};
