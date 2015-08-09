
// CDesktopDimmer: Schnittstelle der Klasse CDesktopDimmer
//

#pragma once


// CDesktopDimmer
//

class CDesktopDimmer : public CWnd
{
public:
	CDesktopDimmer();

	BOOL Create(CWnd* pParentWnd);

protected:
	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
