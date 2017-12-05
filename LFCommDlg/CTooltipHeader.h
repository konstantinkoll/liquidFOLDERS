
// CTooltipHeader: Schnittstelle der Klasse CTooltipHeader
//

#pragma once
#include "CFrontstageWnd.h"


// CTooltipHeader
//

class CTooltipHeader : public CHeaderCtrl
{
public:
	CTooltipHeader();

	virtual void PreSubclassWindow();

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL Shadow=FALSE);
	void SetShadow(BOOL Shadow);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnLayout(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	DECLARE_TOOLTIP()

protected:
	virtual void Init();

	BOOL m_Shadow;
	INT m_PressedItem;
	INT m_TrackItem;
	static CIcons m_SortIndicators;
};
