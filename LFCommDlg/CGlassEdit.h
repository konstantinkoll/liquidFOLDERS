
// CGlassEdit.h: Schnittstelle der Klasse CGlassEdit
//

#pragma once
#include "CGlassWindow.h"


// CGlassEdit
//

class CGlassEdit : public CEdit
{
public:
	CGlassEdit();

	BOOL Create(CGlassWindow* pParentWnd, UINT nID, CString EmptyHint, BOOL ShowSearchIcon=FALSE);
	UINT GetPreferredHeight();

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

	CString m_EmptyHint;
	BOOL m_ShowSearchIcon;
	HICON hSearchIcon;
	INT m_IconSize;
	INT m_ClientAreaTopOffset;
	INT m_FontHeight;
	BOOL m_Hover;
};
