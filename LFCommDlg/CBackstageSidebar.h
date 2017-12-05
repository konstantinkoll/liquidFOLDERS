
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

#define SBI_SELECTABLE        0x01
#define SBI_ENABLED           0x02
#define SBI_VISIBLE           0x04
#define SBI_ALWAYSVISIBLE     0x08
#define SBI_LABELABOVE        0x10
#define SBI_LABELBELOW        0x20
#define SBI_CANFIRE           (SBI_SELECTABLE | SBI_ENABLED | SBI_VISIBLE)

struct SidebarItem
{
	RECT Rect;
	UINT Flags;
	UINT CmdID;
	INT IconID;
	WCHAR Caption[256];
	INT Height;
	UINT Count;
	COLORREF Color;
};

class CBackstageSidebar : public CFrontstageWnd
{
friend class CBackstageWnd;

public:
	CBackstageSidebar();

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL ShowCounts=FALSE);
	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT nID, BOOL ShowCounts);
	BOOL Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID, BOOL ShowCounts);
	void AddCommand(UINT CmdID, INT IconID, LPCWSTR pCaption, COLORREF Color=(COLORREF)-1);
	void AddCommand(UINT CmdID, INT IconID, COLORREF Color=(COLORREF)-1);
	void AddCaption(LPCWSTR pCaption=L"");
	void AddCaption(UINT ResID);
	void UpdateItem(UINT CmdID, UINT Count, BOOL AlwaysVisible=TRUE, COLORREF Color=(COLORREF)-1);
	INT GetPreferredWidth(INT MaxWidth=-1) const;
	INT GetMinHeight() const;
	void SetSelection(UINT CmdID=0);
	static CString FormatCount(UINT Count);

protected:
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual void ShowTooltip(const CPoint& point);

	BOOL IsItemAlwaysVisible(INT Index) const;
	BOOL IsItemEnabled(INT Index) const;
	BOOL IsItemLabel(INT Index) const;
	BOOL IsItemLargeLabel(INT Index) const;
	BOOL IsItemSelectable(INT Index) const;
	BOOL IsItemVisible(INT Index) const;
	BOOL CanItemFire(INT Index) const;
	BOOL HasItemLabelAbove(INT Index) const;
	BOOL HasItemLabelBelow(INT Index) const;
	void AddItem(BOOL Selectable, UINT CmdID, INT IconID, LPCWSTR Caption, COLORREF Color=(COLORREF)-1);
	void SetShadow(BOOL ShowShadow);
	void PressItem(INT Index);
	void AdjustLayout();

	afx_msg LRESULT OnNcHitTest(CPoint point);
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
	INT m_PreferredWidth;
	INT m_SelectedItem;
	INT m_PressedItem;
	INT m_CountWidth;
	BOOL m_Keyboard;
	BOOL m_ShowCounts;
	BOOL m_ShowShadow;
};

inline BOOL CBackstageSidebar::IsItemAlwaysVisible(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_ALWAYSVISIBLE);
}

inline BOOL CBackstageSidebar::IsItemEnabled(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_ENABLED);
}

inline BOOL CBackstageSidebar::IsItemLabel(INT Index) const
{
	return !IsItemSelectable(Index);
}

inline BOOL CBackstageSidebar::IsItemLargeLabel(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return IsItemLabel(Index) && m_Items[Index].Caption[0];

}
inline BOOL CBackstageSidebar::IsItemSelectable(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_SELECTABLE);
}

inline BOOL CBackstageSidebar::IsItemVisible(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_VISIBLE);
}

inline BOOL CBackstageSidebar::CanItemFire(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_CANFIRE)==SBI_CANFIRE;
}

inline BOOL CBackstageSidebar::HasItemLabelAbove(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_LABELABOVE);
}

inline BOOL CBackstageSidebar::HasItemLabelBelow(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<(INT)m_Items.m_ItemCount);

	return (m_Items[Index].Flags & SBI_LABELBELOW);
}

inline void CBackstageSidebar::AddCommand(UINT CmdID, INT IconID, LPCWSTR pCaption, COLORREF Color)
{
	ASSERT(pCaption);

	AddItem(TRUE, CmdID, IconID, pCaption, Color);
}

inline void CBackstageSidebar::AddCaption(LPCWSTR pCaption)
{
	ASSERT(pCaption);

	AddItem(FALSE, 0, -1, pCaption);
}

inline void CBackstageSidebar::SetShadow(BOOL ShowShadow)
{
	if (m_ShowShadow!=ShowShadow)
	{
		m_ShowShadow = ShowShadow;
		Invalidate();
	}
}
