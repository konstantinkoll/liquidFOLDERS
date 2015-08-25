
// CHoverButton.h: Schnittstelle der Klasse CHoverButton
//

#pragma once


// CHoverButton
//

#define WM_ISHOVER     WM_USER+7

class CHoverButton : public CButton
{
public:
	CHoverButton();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg LRESULT OnIsHover(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL m_Hover;
};
