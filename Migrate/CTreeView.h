
// CTreeView.h: Schnittstelle der Klasse CTreeView
//

#pragma once
#include "LFCommDlg.h"


// CTreeView
//

class CTreeView : public CWnd
{
public:
	CTreeView();

	BOOL Create(CWnd* _pParentWnd, UINT nID);
	void AdjustLayout();
	void ClearRoot();
	void SetRoot(LPITEMIDLIST pidl, BOOL Update);

protected:
	CTooltipHeader m_wndHeader;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	UINT m_HeaderHeight;
};
