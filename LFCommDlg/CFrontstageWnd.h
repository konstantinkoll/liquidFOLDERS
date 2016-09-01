
// CFrontstageWnd: Schnittstelle der Klasse CFrontstageWnd
//

#pragma once


// CFrontstageWnd
//

class CFrontstageWnd : public CWnd
{
public:
	void DrawWindowEdge(Graphics& g, BOOL Themed);
	void DrawWindowEdge(CDC& dc, BOOL Themed);

protected:
	afx_msg LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	DECLARE_MESSAGE_MAP()
};
