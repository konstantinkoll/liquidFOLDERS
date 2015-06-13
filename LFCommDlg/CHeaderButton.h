
// CHeaderButton.h: Schnittstelle der Klasse CHeaderButton
//

#pragma once
#include "LFTooltip.h"


// CHeaderButton
//

#define WM_ADJUSTLAYOUT     WM_USER+2
#define WM_GETMENU          WM_USER+3

class CHeaderButton : public CButton
{
public:
	CHeaderButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	BOOL Create(CWnd* pParentWnd, UINT nID, CString Caption, CString Hint);
	void SetValue(CString Value, BOOL ShowDropdown=TRUE, BOOL Repaint=TRUE);
	void GetPreferredSize(CSize& sz, UINT& CaptionWidth);
	void GetCaption(CString& Caption, UINT& CaptionWidth);

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

	CString m_Caption;
	CString m_Hint;
	CString m_Value;
	UINT m_CaptionWidth;
	BOOL m_ShowDropdown;

private:
	LFTooltip m_TooltipCtrl;
	BOOL m_Hover;
};
