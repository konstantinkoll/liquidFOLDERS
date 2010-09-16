
// CTreeView.h: Schnittstelle der Klasse CTreeView
//

#pragma once


// CTreeHeader
//

class CTreeHeader : public CHeaderCtrl
{
public:
	CTreeHeader();

protected:
	afx_msg void OnLButtonDown(UINT NFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT NFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};


// CTreeView
//

class CTreeView : public CWnd
{
public:
	CTreeView();

	int Create(CWnd* _pParentWnd, UINT nID);

protected:
	CTreeHeader m_wndHeader;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
