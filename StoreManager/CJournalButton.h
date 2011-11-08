
// CJournalButton.h: Schnittstelle der Klasse CJournalButton
//

#pragma once
#include "LFCommDlg.h"


// CJournalUI
//

class CJournalUI : public CCmdUI
{
public:
	CJournalUI();

	virtual void Enable(BOOL bOn);

	BOOL m_Enabled;
};


// CJournalButton
//

class CJournalButton : public CWnd
{
public:
	CJournalButton();

	BOOL Create(UINT SuggestedHeight, CGlassWindow* pParentWnd, UINT nID);
	UINT GetPreferredHeight();
	UINT GetPreferredWidth();

protected:
	BOOL m_IsLarge;
	BOOL m_CmdEnabled[2];
	INT m_Hover;
	INT m_Pressed;
	CGdiPlusBitmapResource m_Frame;
	CGdiPlusBitmapResource m_Normal;
	CGdiPlusBitmapResource m_Hot;
	CGdiPlusBitmapResource m_Pushed;
	CGdiPlusBitmapResource m_Disabled;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnIdleUpdateCmdUI();
	DECLARE_MESSAGE_MAP()

private:
	UINT m_ButtonHeight;
	UINT m_ButtonWidth;

	void DrawLeft(Graphics& g, CGdiPlusBitmap* pBmp);
	void DrawRight(Graphics& g, CGdiPlusBitmap* pBmp);
};
