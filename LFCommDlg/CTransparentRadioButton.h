
// CTransparentRadioButton.h: Schnittstelle der Klasse CTransparentRadioButton
//

#pragma once
#include "LFApplication.h"


// CTransparentRadioButton
//

class AFX_EXT_CLASS CTransparentRadioButton : public CButton
{
public:
	CTransparentRadioButton();
	virtual ~CTransparentRadioButton();

protected:
	LFApplication* p_App;
	HTHEME hTheme;

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_Hover;
};
