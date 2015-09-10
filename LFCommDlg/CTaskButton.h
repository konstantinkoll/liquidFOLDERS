
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once
#include "CHoverButton.h"


// CTaskButton
//

class CTaskButton : public CHoverButton
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID, CString Caption, CString Hint, CMFCToolBarImages* pButtonIcons, CMFCToolBarImages* pTooltipIcons, INT IconSize, INT IconID, BOOL ForceSmall, BOOL HideIcon);
	void SetIconID(INT IconID, INT OverlayID=-1);
	INT GetPreferredWidth(BOOL Small=FALSE);

protected:
	afx_msg void OnPaint();
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	CMFCToolBarImages* p_ButtonIcons;
	CMFCToolBarImages* p_TooltipIcons;
	CString m_Caption;
	CString m_Hint;
	BOOL m_Small;
	INT m_IconID;
	INT m_OverlayID;

private:
	INT m_IconSize;
	BOOL m_ForceSmall;
	BOOL m_HideIcon;
};
