
// CToolbarList: Schnittstelle der Klasse CToolbarList
//

#pragma once
#include "liquidFOLDERS.h"
#include "CPaneList.h"


// CToolbarList
//

class AFX_EXT_CLASS CToolbarList : public CPaneList
{
public:
	CToolbarList();
	~CToolbarList();

	virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags);

protected:
	void SetSpacing(int cx);

	int m_Width;

	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	DECLARE_MESSAGE_MAP()
};
