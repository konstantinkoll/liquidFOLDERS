
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once
#include "CHoverButton.h"


// CTaskButton
//

class CTaskButton : public CHoverButton
{
public:
	CTaskButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID, CString Caption, CString TooltipHeader, CString TooltipHint, CMFCToolBarImages* Icons=NULL, INT IconSize=0, INT IconID=-1);
	void SetIconID(INT IconID, INT OverlayID=-1);
	INT GetPreferredWidth();

protected:
	afx_msg void OnPaint();
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_TooltipHeader;
	CString m_TooltipHint;
	CMFCToolBarImages* p_Icons;
	INT m_IconSize;
	INT m_IconID;
	INT m_OverlayID;
};
