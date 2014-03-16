
// CBottomArea.h: Schnittstelle der Klasse CBottomArea
//

#pragma once
#include "LFApplication.h"


// CBottomArea
//

class AFX_EXT_CLASS CBottomArea : public CDialogBar
{
public:
	CBottomArea();

protected:
	LFApplication* p_App;

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
