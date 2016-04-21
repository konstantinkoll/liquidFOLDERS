
// CBackstageSidebar.h: Schnittstelle der Klasse CBackstageSidebar
//

#pragma once
#include "CFrontstageWnd.h"
#include "CIcons.h"
#include "LFDynArray.h"


// CSidebarCmdUI
//

class CSidebarCmdUI : public CCmdUI
{
public:
	CSidebarCmdUI();

	virtual void Enable(BOOL bOn=TRUE);

	BOOL m_Enabled;
};


// CBackstageSidebar
//

struct SidebarItem
{
	RECT Rect;
	BOOL Selectable;
	BOOL Enabled;
	UINT CmdID;
	INT IconID;
	WCHAR Caption[256];
	INT Height;
	UINT Count;
	COLORREF Color;
};

class CBackstageSidebar : public CFrontstageWnd
{
public:
	CBackstageSidebar();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void AdjustLayout();

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL ShowCounts=FALSE);
	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT nID, BOOL ShowCounts=FALSE);
	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID, BOOL ShowCounts=FALSE);
	void AddCommand(UINT CmdID, INT IconID, LPCWSTR Caption, COLORREF Color=(COLORREF)-1);
	void AddCaption(LPCWSTR Caption=L"");
	void AddCaption(UINT ResID);
	void ResetCounts();
	void SetCount(UINT CmdID, UINT Count);
	INT GetPreferredWidth() const;
	INT GetMinHeight() const;
	void SetSelection(UINT CmdID=0);

protected:
	void AddItem(BOOL Selectable, UINT CmdID, INT IconID, LPCWSTR Caption, COLORREF Color=(COLORREF)-1);
	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT Index);
	void PressItem(INT Index);

	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

	CIcons* p_ButtonIcons;
	CIcons* p_TooltipIcons;
	INT m_IconSize;
	LFDynArray<SidebarItem, 8, 8> m_Items;
	INT m_Width;
	INT m_SelectedItem;
	INT m_HotItem;
	INT m_PressedItem;
	INT m_CountWidth;
	BOOL m_Hover;
	BOOL m_Keyboard;
	BOOL m_ShowCounts;
};

inline INT CBackstageSidebar::GetPreferredWidth() const
{
	return m_Width;
}
