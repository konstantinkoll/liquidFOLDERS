
// CBackstageSidebar.h: Schnittstelle der Klasse CBackstageSidebar
//

#pragma once
#include "CFrontstageWnd.h"
#include "CIcons.h"
#include "LFCore.h"


// CBackstageSidebar
//

struct SidebarItem
{
	RECT Rect;
	BOOL Selectable;
	UINT CmdID;
	INT IconID;
	WCHAR Caption[256];
	WCHAR Hint[256];
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
	virtual CString AppendTooltip(UINT CmdID);

	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, UINT LargeResID, CIcons& SmallIcons, UINT SmallResID, UINT nID, BOOL ShowCounts=FALSE);
	void AddCommand(UINT CmdID, INT IconID, LPCWSTR Caption, LPCWSTR Hint, COLORREF Color=(COLORREF)-1);
	void AddCaption(LPCWSTR Caption=NULL);
	void AddCaption(UINT ResID);
	void ResetCounts();
	void SetCount(UINT CmdID, UINT Count);
	INT GetPreferredWidth();
	INT GetMinHeight();
	void SetSelection(UINT CmdID=0);

protected:
	void AddItem(BOOL Selectable, UINT CmdID, INT IconID, LPCWSTR Caption, LPCWSTR Hint, COLORREF Color=(COLORREF)-1);
	INT ItemAtPosition(CPoint point);
	void InvalidateItem(INT Index);
	void SelectItem(INT Index);

	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

	CIcons* p_ButtonIcons;
	CIcons* p_TooltipIcons;
	INT m_IconSize;
	LFDynArray<SidebarItem, 8, 8> m_Items;
	INT m_Width;
	INT m_SelectedItem;
	INT m_HotItem;
	INT m_CountWidth;
	BOOL m_Hover;
	BOOL m_Keyboard;
	BOOL m_ShowCounts;
};
