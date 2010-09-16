
// CTreeView.h: Schnittstelle der Klasse CTreeView
//

#pragma once


// CTreeView
//

class CTreeView : public CWnd
{
public:
	CTreeView();

	int Create(CWnd* _pParentWnd, UINT nID);

protected:
	CHeaderCtrl m_wndHeader;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};
