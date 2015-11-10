
// CHeaderArea.h: Schnittstelle der Klasse CHeaderArea
//

#pragma once
#include "CFrontstageWnd.h"
#include "CHeaderButton.h"
#include "LFCore.h"


// CHeaderArea
//

class CHeaderArea : public CFrontstageWnd
{
public:
	CHeaderArea();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL Shadow=FALSE);
	void SetText(LPCWSTR Caption, LPCWSTR Hint, BOOL Repaint=TRUE);
	UINT GetPreferredHeight();
	CHeaderButton* AddButton(UINT nID=0);

protected:
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

	BOOL m_Shadow;
	LFDynArray<CHeaderButton*, 2, 2> m_Buttons;
	CString m_Caption;
	CString m_Hint;
	INT m_RightEdge;

private:
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
