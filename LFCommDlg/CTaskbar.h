
// CTaskbar.h: Schnittstelle der Klasse CTaskbar
//

#pragma once
#include "CTaskButton.h"
#include <list>


// CTaskbar
//

class AFX_EXT_CLASS CTaskbar : public CWnd
{
public:
	CTaskbar();

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, UINT ResID, UINT nID);
	UINT GetPreferredHeight();
	CTaskButton* AddButton(UINT nID, INT IconID, BOOL ForceIcon=FALSE, BOOL AddRight=FALSE);
	void AdjustLayout();

protected:
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnThemeChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	CMFCToolBarImages Icons;
	CList<CTaskButton*> m_ButtonsLeft;
	CList<CTaskButton*> m_ButtonsRight;
	CBitmap BackBuffer;
	INT BackBufferL;
	INT BackBufferH;
	HBRUSH hBackgroundBrush;
};
