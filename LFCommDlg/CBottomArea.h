
// CBottomArea.h: Schnittstelle der Klasse CBottomArea
//

#pragma once


// CBottomArea
//

class AFX_EXT_CLASS CBottomArea : public CDialogBar
{
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};
