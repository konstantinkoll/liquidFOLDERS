
// CExplorerHeader.h: Schnittstelle der Klasse CExplorerHeader
//

#pragma once
#include "CHeaderButton.h"


// CExplorerHeader
//

class AFX_EXT_CLASS CExplorerHeader : public CWnd
{
public:
	CExplorerHeader();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetText(CString Caption, CString Hint, BOOL Repaint=TRUE);
	void SetColors(COLORREF CaptionCol, COLORREF HintCol=(COLORREF)-1, BOOL Repaint=TRUE);
	void SetLineStyle(BOOL GradientLine, BOOL Repaint=TRUE);
	UINT GetPreferredHeight();
	CHeaderButton* AddButton(UINT nID);

protected:
	CList<CHeaderButton*> m_Buttons;
	COLORREF m_CaptionCol;
	COLORREF m_HintCol;
	CString m_Caption;
	CString m_Hint;
	CBitmap m_Background;
	HBRUSH hBackgroundBrush;
	BOOL m_GradientLine;
	UINT m_RightEdge;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnAdjustLayout();
	DECLARE_MESSAGE_MAP()

private:
	LFApplication* p_App;
	UINT m_FontHeight;
};
