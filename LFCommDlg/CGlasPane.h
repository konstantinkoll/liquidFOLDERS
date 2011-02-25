
// CGlasPane: Schnittstelle der Klasse CGlasPane
//

#pragma once


// CGlasPane
//

class AFX_EXT_CLASS CGlasPane : public CWnd
{
public:
	CGlasPane();

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
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()
};
