
// CGlasPane: Schnittstelle der Klasse CGlasPane
//

#pragma once


// CGlasPane
//

class AFX_EXT_CLASS CGlasPane : public CWnd
{
public:
	CGlasPane(BOOL IsLeft);

	BOOL Create(CWnd* pParentWnd, UINT nID);

protected:
	BOOL m_IsLeft;

	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnNcPaint();
	DECLARE_MESSAGE_MAP()
};
