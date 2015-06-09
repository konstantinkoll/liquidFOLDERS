
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once
#include "LFTooltip.h"


// CTaskButton
//

class CTaskButton : public CButton
{
public:
	CTaskButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	BOOL Create(CString Caption, CString TooltipHeader, CString TooltipHint, CMFCToolBarImages* Icons, INT IconSize, INT IconID, CWnd* pParentWnd, UINT nID);
	void SetIconID(INT IconID, INT OverlayID=-1);
	INT GetPreferredWidth();

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_TooltipHeader;
	CString m_TooltipHint;
	LFTooltip m_TooltipCtrl;
	CMFCToolBarImages* p_Icons;
	INT m_IconSize;
	INT m_IconID;
	INT m_OverlayID;
	BOOL m_Hover;
};
