
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once
#include "CHoverButton.h"


// CTaskButton
//

class CTaskButton : public CHoverButton
{
public:
	BOOL Create(CWnd* pParentWnd, UINT nID, const CString& Caption, const CString& Hint, CIcons* pButtonIcons, CIcons* pTooltipIcons, INT IconSize, INT IconID, BOOL ForceSmall, BOOL HideIcon);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	void SetIconID(INT IconID);
	INT GetPreferredWidth(BOOL Small=FALSE);

protected:
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CIcons* p_ButtonIcons;
	CIcons* p_TooltipIcons;
	CString m_Caption;
	CString m_Hint;
	BOOL m_Small;
	INT m_IconID;

private:
	INT m_IconSize;
	BOOL m_ForceSmall;
	BOOL m_HideIcon;
};
