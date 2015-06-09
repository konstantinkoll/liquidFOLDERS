
// CGlassPane: Schnittstelle der Klasse CGlassPane
//

#pragma once


// CGlassPane
//

class CGlassPane : public CWnd
{
public:
	CGlassPane();

	virtual void AdjustLayout();

	BOOL Create(BOOL IsLeft, INT PreferredWidth, CWnd* pParentWnd, UINT nID);
	INT GetPreferredWidth();
	void SetMaxWidth(INT MaxWidth);

protected:
	BOOL m_IsLeft;
	INT m_PreferredWidth;
	INT m_MaxWidth;

	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnChildActivate();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};
