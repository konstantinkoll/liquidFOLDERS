
// CJournalButton.h: Schnittstelle der Klasse CJournalButton
//

#pragma once
#include "LFCommDlg.h"


// CJournalButton
//

class CJournalButton : public CWnd
{
public:
	CJournalButton();

	BOOL Create(UINT SuggestedHeight, CGlasWindow* pParentWnd, UINT nID);
	UINT GetPreferredHeight();
	UINT GetPreferredWidth();

protected:
	BOOL m_IsLarge;
	CGdiPlusBitmapResource m_Frame;
	CGdiPlusBitmapResource m_Normal;
	CGdiPlusBitmapResource m_Hot;
	CGdiPlusBitmapResource m_Pressed;
	CGdiPlusBitmapResource m_Disabled;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	void DrawLeft(Graphics& g, CGdiPlusBitmap* pGdiPlusBitmap);
	void DrawRight(Graphics& g, CGdiPlusBitmap* pGdiPlusBitmap);
};
