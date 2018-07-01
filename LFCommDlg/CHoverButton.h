
// CHoverButton.h: Schnittstelle der Klasse CHoverButton
//

#pragma once
#include "CFrontstageWnd.h"


// CHoverButton
//

class CHoverButton : public CButton
{
public:
	CHoverButton();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	BOOL Create(LPCTSTR lpszCaption, CWnd* pParentWnd, UINT nID);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	DECLARE_TOOLTIP()

private:
	BOOL m_DrawBorder;
};
