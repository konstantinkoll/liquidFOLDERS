
// CGlasEdit.h: Schnittstelle der Klasse CGlasEdit
//

#pragma once
#include "CGlasWindow.h"


// CGlasEdit
//

class AFX_EXT_CLASS CGlasEdit : public CEdit
{
public:
	CGlasEdit();

	BOOL Create(CString EmptyHint, CGlasWindow* pParentWnd, UINT nID, BOOL ShowSearchIcon=FALSE);

protected:
	CString m_EmptyHint;
	BOOL m_ShowSearchIcon;
	HICON hSearchIcon;
	INT m_IconSize;
	BOOL m_Hover;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	DECLARE_MESSAGE_MAP()
};
