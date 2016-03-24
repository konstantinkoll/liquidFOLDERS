
// CTooltipHeader: Schnittstelle der Klasse CTooltipHeader
//

#pragma once


// CTooltipHeader
//

class CTooltipHeader : public CHeaderCtrl
{
public:
	CTooltipHeader();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL Shadow=FALSE);
	void SetShadow(BOOL Shadow);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
	virtual void Init();

	BOOL m_Shadow;
	BOOL m_Hover;
	INT m_HoverItem;
	INT m_PressedItem;
	INT m_TrackItem;
	INT m_TooltipItem;
	static CIcons m_SortIndicators;
};
