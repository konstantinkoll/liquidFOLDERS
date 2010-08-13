
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once
#include "CSimpleTooltip.h"


// CTaskButton
//

class AFX_EXT_CLASS CTaskButton : public CButton
{
public:
	CTaskButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CString Caption, CString Tooltip, CMFCToolBarImages* Icons, int IconID, CWnd* pParentWnd, UINT nID);
	int GetPreferredWidth();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_Tooltip;
	CSimpleTooltip m_TooltipCtrl;
	CMFCToolBarImages* m_Icons;
	int m_IconID;
	BOOL m_Hover;
};
