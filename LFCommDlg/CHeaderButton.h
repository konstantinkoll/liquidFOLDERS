
// CHeaderButton.h: Schnittstelle der Klasse CHeaderButton
//

#pragma once
#include "LFTooltip.h"


// CHeaderButton
//

#define WM_ADJUSTLAYOUT     WM_USER+8
#define WM_GETMENU          WM_USER+9

class AFX_EXT_CLASS CHeaderButton : public CButton
{
public:
	CHeaderButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	BOOL Create(CString Caption, CString Hint, CWnd* pParentWnd, UINT nID);
	void SetValue(CString Value);
	void GetPreferredSize(CSize& sz);

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_Hint;
	CString m_Value;
	LFTooltip m_TooltipCtrl;
	BOOL m_Hover;
};
