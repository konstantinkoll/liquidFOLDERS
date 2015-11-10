
// CBackstageEdit.h: Schnittstelle der Klasse CBackstageEdit
//

#pragma once
#include "CBackstageWnd.h"


// CBackstageEdit
//

class CBackstageEdit : public CEdit
{
public:
	CBackstageEdit();

	BOOL Create(CBackstageWnd* pParentWnd, UINT nID, CString EmptyHint);

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	CString m_EmptyHint;
	INT m_ClientAreaTopOffset;
	BOOL m_Hover;
};
