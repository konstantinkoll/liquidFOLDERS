
// CFrontstageItemView: Schnittstelle der Klasse CFrontstageItemView
//

#pragma once
#include "CBackstageWnd.h"
#include "CFrontstageScroller.h"
#include "LFDynArray.h"
#include "resource.h"


// CFrontstageItemView
//

#define FRONTSTAGE_ENABLESCROLLING          0x00000100
#define FRONTSTAGE_ENABLEFOCUSITEM          0x00000200
#define FRONTSTAGE_ENABLESELECTION          0x00000400
#define FRONTSTAGE_ENABLESHIFTSELECTION     0x00000800
#define FRONTSTAGE_ENABLEDRAGANDDROP        0x00001000
#define FRONTSTAGE_ENABLELABELEDIT          0x00002000

#define IVN_SELECTIONCHANGED                0x00000100
#define IVN_BEGINDRAGANDDROP                0x00000200

#define ITEMCELLPADDING          2
#define ITEMVIEWICONPADDING      4
#define ITEMVIEWMARGIN           5
#define ITEMVIEWMARGINLARGE      (BACKSTAGEBORDER-2)
#define ITEMVIEWPADDING          (BACKSTAGEBORDER-ITEMVIEWMARGIN)

#define ITEMVIEWMINWIDTH     64

#define BITMAP_SELECTION      0
#define BITMAP_REFLECTION     1

#define SETFOCUSITEM_MOVEONLY             0
#define SETFOCUSITEM_MOVEDESELECT         1
#define SETFOCUSITEM_MOVETOGGLESELECT     2
#define SETFOCUSITEM_SHIFTSELECT          3

struct CachedSelectionBitmap
{
	HBITMAP hBitmap;
	INT Width;
	INT Height;
};

struct ItemCategoryData
{
	RECT Rect;
	WCHAR Caption[256];
	WCHAR Hint[256];
	INT IconID;
};

struct ItemData
{
	RECT Rect;
	BOOL Valid;
	INT Column;
	INT Row;
};

class CFrontstageItemView : public CFrontstageScroller
{
public:
	CFrontstageItemView(UINT Flags=FRONTSTAGE_ENABLESCROLLING, SIZE_T szData=sizeof(ItemData), const CSize& szItemInflate=CSize(0, 0));

	BOOL Create(CWnd* pParentWnd, UINT nID, const CRect& rect=CRect(0, 0, 0, 0), UINT nClassStyle=0);
	INT GetSelectedItem() const;
	void SetFocusItem(INT Index, UINT Mode=SETFOCUSITEM_MOVEDESELECT);
	void SelectNone();

protected:
	virtual INT GetItemCategory(INT Index) const;
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual COLORREF GetItemTextColor(INT Index) const;
	virtual BOOL IsItemSelected(INT Index) const;
	virtual void SelectItem(INT Index, BOOL Select=TRUE);
	virtual INT HandleNavigationKeys(UINT nChar, BOOL Control) const;
	virtual void FireSelectedItem() const;
	virtual void DeleteSelectedItem() const;
	virtual void AdjustScrollbars();
	virtual BOOL DrawNothing() const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);

	LRESULT SendNotifyMessage(UINT Code) const;
	BOOL IsScrollingEnabled() const;
	BOOL IsFocusItemEnabled() const;
	BOOL IsSelectionEnabled() const;
	BOOL IsShiftSelectionEnabled() const;
	BOOL IsDragAndDropEnabled() const;
	BOOL IsLabelEditEnabled() const;
	void AddItemCategory(LPCWSTR Caption, LPCWSTR Hint=L"", INT IconID=0);
	void SetItemCount(UINT ItemCount, BOOL Virtual);
	void AddItem(LPCVOID pData);
	void LastItem();
	ItemData* GetItemData(INT Index) const;
	void FreeItemData(BOOL InternalCall=FALSE);
	void ValidateAllItems();
	void SortItems(PFNCOMPARE zCompare, UINT Attr=0, BOOL Descending=FALSE, BOOL Parameter1=FALSE, BOOL Parameter2=FALSE);
	RECT GetItemRect(INT Index) const;
	BOOL HasItemsSelected() const;
	void ItemSelectionChanged(INT Index);
	void ItemSelectionChanged();
	void EnsureVisible(INT Index);
	void ResetDragLocation();
	BOOL IsDragLocationValid() const;
	void DrawTile(CDC& dc, CRect rect, CIcons& Icons, INT IconID, COLORREF TextColor, UINT Rows, ...) const;
	void DrawTile(CDC& dc, CRect rect, CImageList& ImageList, INT IconID, UINT nStyle, COLORREF TextColor, UINT Rows, ...) const;
	void DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	void DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	COLORREF GetLightTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF GetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF SetLightTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF SetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const;
	void SetItemHeight(INT IconSize, INT Rows, INT Padding=ITEMVIEWPADDING);
	void SetItemHeight(const CIcons& Icons, INT Rows, INT Padding=ITEMVIEWPADDING);
	void SetItemHeight(const CImageList& ImageList, INT Rows, INT Padding=ITEMVIEWPADDING);
	void GetLayoutRect(CRect& rectLayout);
	void AdjustLayoutGrid(const CSize& szItem, BOOL FullWidth=FALSE, INT Margin=ITEMVIEWMARGIN);
	void AdjustLayoutColumns(INT Columns=1, INT Margin=ITEMVIEWMARGIN);
	void AdjustLayoutSingleRow(INT Columns);

	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnSelectToggle();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	INT m_LargeFontHeight;
	INT m_DefaultFontHeight;
	INT m_SmallFontHeight;
	INT m_IconSize;

	CSize m_szItemInflate;

	LFDynArray<ItemCategoryData, 4, 4> m_ItemCategories;

	INT m_ItemCount;
	BOOL m_Nothing;

	INT m_FocusItem;
	BOOL m_FocusItemSelected;
	INT m_EditItem;
	INT m_SelectionAnchor;
	BOOL m_MultipleSelected;
	BOOL m_ShowFocusRect;

private:
	void ResetItemCategories();
	static INT GetGutterForMargin(INT Margin);
	static UINT GetTileRows(UINT Rows, va_list vl);
	void DrawTile(CDC& dc, CRect& rect, COLORREF TextColor, UINT Rows, va_list& vl) const;

	SIZE_T m_szData;
	LPBYTE m_pItemData;
	UINT m_ItemDataAllocated;
	CPoint m_DragPos;

	CachedSelectionBitmap m_Bitmaps[2];
};

inline BOOL CFrontstageItemView::IsScrollingEnabled() const
{
	return (m_Flags & FRONTSTAGE_ENABLESCROLLING);
}

inline BOOL CFrontstageItemView::IsFocusItemEnabled() const
{
	return (m_Flags & FRONTSTAGE_ENABLEFOCUSITEM);
}

inline BOOL CFrontstageItemView::IsSelectionEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLESELECTION)==0) || (m_Flags & FRONTSTAGE_ENABLEFOCUSITEM));

	return (m_Flags & FRONTSTAGE_ENABLESELECTION);
}

inline BOOL CFrontstageItemView::IsShiftSelectionEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLESELECTION)==0) || (m_Flags & FRONTSTAGE_ENABLEFOCUSITEM));
	ASSERT(((m_Flags & FRONTSTAGE_ENABLESHIFTSELECTION)==0) || (m_Flags & FRONTSTAGE_ENABLESELECTION));

	return (m_Flags & FRONTSTAGE_ENABLESHIFTSELECTION);
}

inline BOOL CFrontstageItemView::IsDragAndDropEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLEDRAGANDDROP)==0) || (m_Flags & FRONTSTAGE_ENABLEFOCUSITEM));

	return (m_Flags & FRONTSTAGE_ENABLEDRAGANDDROP);
}

inline BOOL CFrontstageItemView::IsLabelEditEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLELABELEDIT)==0) || (m_Flags & FRONTSTAGE_ENABLEFOCUSITEM));

	return (m_Flags & FRONTSTAGE_ENABLELABELEDIT);
}

inline void CFrontstageItemView::ResetItemCategories()
{
	for (UINT a=0; a<m_ItemCategories.m_ItemCount; a++)
		ZeroMemory(&m_ItemCategories[a].Rect, sizeof(RECT));
}

inline ItemData* CFrontstageItemView::GetItemData(INT Index) const
{
	ASSERT(m_pItemData);
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	return (ItemData*)(m_pItemData+Index*m_szData);
}

inline INT CFrontstageItemView::GetGutterForMargin(INT Margin)
{
	return (Margin>=BACKSTAGEBORDER) ? ITEMVIEWMARGINLARGE : ITEMVIEWMARGIN;
}

inline void CFrontstageItemView::SelectNone()
{
	SendMessage(WM_COMMAND, IDM_ITEMVIEW_SELECTNONE);
}

inline void CFrontstageItemView::ItemSelectionChanged(INT Index)
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	InvalidateItem(Index);
	SendNotifyMessage(IVN_SELECTIONCHANGED);
}

inline void CFrontstageItemView::ItemSelectionChanged()
{
	Invalidate();
	SendNotifyMessage(IVN_SELECTIONCHANGED);
}

inline void CFrontstageItemView::ResetDragLocation()
{
	m_DragPos.x = m_DragPos.y = -1;
}

inline BOOL CFrontstageItemView::IsDragLocationValid() const
{
	return (m_DragPos.x!=-1) && (m_DragPos.y!=-1);
}

inline COLORREF CFrontstageItemView::SetLightTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return dc.SetTextColor(GetLightTextColor(dc, Index, Themed));
}

inline COLORREF CFrontstageItemView::SetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return dc.SetTextColor(GetDarkTextColor(dc, Index, Themed));
}

inline void CFrontstageItemView::SetItemHeight(const CIcons& Icons, INT Rows, INT Padding)
{
	ASSERT(Rows>=1);
	ASSERT(Padding>=0);

	SetItemHeight(Icons.GetIconSize(), Rows, Padding);
}

inline void CFrontstageItemView::SetItemHeight(const CImageList& ImageList, INT Rows, INT Padding)
{
	ASSERT(Rows>=1);
	ASSERT(Padding>=0);

	INT cx;
	INT cy;
	ImageList_GetIconSize(ImageList, &cx, &cy);

	SetItemHeight(cy, Rows, Padding);
}
