
// LFTooltip.h: Schnittstelle der Klasse LFTooltip
//

#pragma once


// LFTooltip
//

#define LFHOVERTIME     850

class LFTooltip : public CWnd
{
public:
	LFTooltip();

	BOOL Create();

	void ShowTooltip(const CPoint& point, const CString& strCaption, const CString& strText, HICON hIcon=NULL, HBITMAP hBitmap=NULL);
	void HideTooltip();

protected:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	CRect m_ContentRect;
};
