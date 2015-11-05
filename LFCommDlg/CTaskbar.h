
// CTaskbar.h: Schnittstelle der Klasse CTaskbar
//

#pragma once
#include "CTaskButton.h"


// CTaskbar
//

class CTaskbar : public CWnd
{
public:
	CTaskbar();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, UINT LargeResID, UINT SmallResID, UINT nID);
	UINT GetPreferredHeight();
	CTaskButton* AddButton(UINT nID, INT IconID, BOOL ForceIcon=FALSE, BOOL AddRight=FALSE, BOOL ForceSmall=FALSE);
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
	CIcons m_ButtonIcons;
	CIcons m_TooltipIcons;
	INT m_IconSize;
	CList<CTaskButton*> m_ButtonsLeft;
	CList<CTaskButton*> m_ButtonsRight;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
