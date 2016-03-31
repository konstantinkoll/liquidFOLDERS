
// CHoverButton.h: Schnittstelle der Klasse CHoverButton
//

#pragma once


// CHoverButton
//

class CHoverButton : public CButton
{
public:
	CHoverButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(LPCTSTR lpszCaption, CWnd* pParentWnd, UINT nID);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	BOOL m_Hover;
};
