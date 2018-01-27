
// CTaskbar.h: Schnittstelle der Klasse CTaskbar
//

#pragma once
#include "CFrontstageWnd.h"
#include "CTaskButton.h"
#include "LFDynArray.h"


// CTaskbar
//

class CTaskbar : public CFrontstageWnd
{
public:
	CTaskbar();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID);
	UINT GetPreferredHeight() const;
	CTaskButton* AddButton(UINT nID, INT IconID, BOOL ForceIcon=FALSE, BOOL AddRight=FALSE, BOOL ForceSmall=FALSE);
	static void DrawTaskbarShadow(Graphics& g, const CRect& rectClient);

protected:
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);
	virtual void AdjustLayout();

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnThemeChanged();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnIdleUpdateCmdUI();
	DECLARE_MESSAGE_MAP()

private:
	CIcons* p_ButtonIcons;
	CIcons* p_TooltipIcons;
	UINT m_FirstRight;
	LFDynArray<CTaskButton*, 8, 8> m_Buttons;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
};
