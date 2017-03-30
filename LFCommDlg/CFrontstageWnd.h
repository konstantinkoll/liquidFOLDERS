
// CFrontstageWnd: Schnittstelle der Klasse CFrontstageWnd
//

#pragma once


// CFrontstageWnd
//

#define CARDPADDING     7

class CFrontstageWnd : public CWnd
{
public:
	void DrawCardBackground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed);
	void DrawCardForeground(CDC& dc, Graphics& g, LPCRECT lpRect, BOOL Themed, BOOL Hot=FALSE, BOOL Focused=FALSE, BOOL Selected=FALSE, COLORREF TextColor=(COLORREF)-1, BOOL ShowFocusRect=TRUE);
	void DrawWindowEdge(Graphics& g, BOOL Themed);
	void DrawWindowEdge(CDC& dc, BOOL Themed);

protected:
	afx_msg LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	DECLARE_MESSAGE_MAP()
};
