
// CTaskbar.h: Schnittstelle der Klasse CTaskbar
//

#pragma once
#include "CFrontstageWnd.h"
#include "CTaskButton.h"
#include "LFCore.h"


// CTaskbar
//

class CTaskbar : public CFrontstageWnd
{
public:
	CTaskbar();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, UINT LargeResID, CIcons& SmallIcons, UINT SmallResID, UINT nID);
	UINT GetPreferredHeight() const;
	CTaskButton* AddButton(UINT nID, INT IconID, BOOL ForceIcon=FALSE, BOOL AddRight=FALSE, BOOL ForceSmall=FALSE);
	void AdjustLayout();
	static void DrawTaskbarShadow(Graphics& g, const CRect& rectClient);

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
	DECLARE_MESSAGE_MAP()

private:
	CIcons* p_ButtonIcons;
	CIcons* p_TooltipIcons;
	INT m_IconSize;
	UINT m_FirstRight;
	LFDynArray<CTaskButton*, 8, 8> m_Buttons;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
