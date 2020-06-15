
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
#define FRONTSTAGE_ENABLEEDITONHOVER        0x00004000

#define FRONTSTAGE_HIDESELECTIONONEDIT      0x01000000

#define IVN_SELECTIONCHANGED                0x00000100
#define IVN_BEGINDRAGANDDROP                0x00000200

#define ITEMVIEWICONPADDING     4
#define ITEMVIEWMARGIN          5
#define ITEMVIEWMARGINLARGE     (BACKSTAGEBORDER-2)
#define ITEMVIEWPADDING         (BACKSTAGEBORDER-ITEMVIEWMARGIN)
#define ITEMVIEWMINWIDTH        64

#define ITEMCELLPADDINGY     2
#define ITEMCELLPADDINGX     (2*ITEMCELLPADDINGY)
#define ITEMCELLSPACER       (2*ITEMCELLPADDINGX+1)

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
	
	UINT GetItemCount() const;
	void SetFocusItem(INT Index, UINT Mode=SETFOCUSITEM_MOVEDESELECT);
	INT GetSelectedItem() const;
	void SelectNone();
	void EditLabel(INT Index);

protected:
	virtual void AdjustScrollbars();
	virtual INT GetItemCategory(INT Index) const;
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual COLORREF GetItemTextColor(INT Index, BOOL Themed) const;
	virtual INT HandleNavigationKeys(UINT nChar, BOOL Control) const;
	virtual BOOL IsItemSelected(INT Index) const;
	virtual void SelectItem(INT Index, BOOL Select=TRUE);
	virtual void FireSelectedItem();
	virtual void DeleteSelectedItem();
	virtual BOOL DrawNothing() const;
	virtual void DrawItemCell(CDC& dc, CRect& rectCell, INT Index, ATTRIBUTE Attr, BOOL Themed);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual BOOL AllowItemEditLabel(INT Index) const;
	virtual RECT GetLabelRect() const;
	virtual CEdit* CreateLabelEditControl();
	virtual void EndLabelEdit(INT Index, CString& Value);
	virtual void DestroyEdit(BOOL Accept=FALSE);

	BOOL IsScrollingEnabled() const;
	BOOL IsFocusItemEnabled() const;
	BOOL IsSelectionEnabled() const;
	BOOL IsShiftSelectionEnabled() const;
	BOOL IsDragAndDropEnabled() const;
	BOOL IsLabelEditEnabled() const;
	BOOL IsEditOnHoverEnabled() const;
	BOOL IsHideSelectionOnEditEnabled() const;
	LRESULT SendNotifyMessage(UINT Code) const;
	void SetItemHeight(INT IconSize, INT Rows, INT Padding=ITEMVIEWPADDING);
	void SetItemHeight(const CIcons& Icons, INT Rows, INT Padding=ITEMVIEWPADDING);
	void SetItemHeight(const CImageList& ImageList, INT Rows, INT Padding=ITEMVIEWPADDING);
	void EnsureVisible(INT Index);
	void GetLayoutRect(CRect& rectLayout);
	void AdjustLayoutGrid(const CSize& szItem, BOOL FullWidth=FALSE, INT Margin=ITEMVIEWMARGIN, INT TopOffset=0, BOOL AllowItemSeparator=TRUE);
	void AdjustLayoutList();
	void AdjustLayoutColumns(INT Columns=1, INT Margin=ITEMVIEWMARGIN);
	void AdjustLayoutSingleRow(INT Columns, INT Margin=ITEMVIEWMARGIN);
	void AddItemCategory(LPCWSTR Caption, LPCWSTR Hint=L"", INT IconID=0);
	void SetItemCount(UINT ItemCount, BOOL Virtual);
	void AddItem(LPCVOID pData);
	void LastItem();
	void ValidateAllItems();
	ItemData* GetItemData(INT Index) const;
	void FreeItemData(BOOL InternalCall=FALSE);
	void SortItems(PFNCOMPARE zCompare, ATTRIBUTE Attr=0, BOOL Descending=FALSE, BOOL Parameter1=FALSE, BOOL Parameter2=FALSE);
	RECT GetItemRect(INT Index) const;
	BOOL HasItemsSelected() const;
	void ItemSelectionChanged(INT Index);
	void ItemSelectionChanged();
	void ResetDragLocation();
	BOOL IsDragLocationValid() const;
	void DrawItemSeparator(CDC& dc, LPCRECT rectItem, BOOL Themed);
	void DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	void DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	COLORREF GetLightTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF GetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF SetLightTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF SetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const;
	void DrawCountItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, LPCWSTR Label, UINT Count);
	void DrawListItem(CDC& dc, CRect rect, INT Index, BOOL Themed, INT* pColumnOrder, INT* pColumnWidth, INT PreviewAttribute=-1);
	void DrawTile(CDC& dc, CRect rect, CIcons& Icons, INT IconID, COLORREF TextColor, UINT Rows, ...) const;
	void DrawTile(CDC& dc, CRect rect, CImageList& ImageList, INT IconID, UINT nStyle, COLORREF TextColor, UINT Rows, ...) const;

	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
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
	BOOL m_DrawItemSeparator;

private:
	static INT GetGutterForMargin(INT Margin);
	void ResetItemCategories();
	BOOL IsEditItem(INT Index) const;
	BOOL DrawHover(INT Index) const;
	BOOL DrawSelection(INT Index) const;
	static UINT GetTileRows(UINT Rows, va_list vl);
	void DrawTile(CDC& dc, CRect& rect, COLORREF TextColor, UINT Rows, va_list& vl) const;

	SIZE_T m_szData;
	LPBYTE m_pItemData;
	UINT m_ItemDataAllocated;

	static INT m_MaxCountWidth;

	CPoint m_DragPos;
	BOOL m_ButtonDownInWindow;

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
	ASSERT(((m_Flags & FRONTSTAGE_ENABLESELECTION)==0) || IsFocusItemEnabled());

	return (m_Flags & FRONTSTAGE_ENABLESELECTION);
}

inline BOOL CFrontstageItemView::IsShiftSelectionEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLESHIFTSELECTION)==0) || IsSelectionEnabled());

	return (m_Flags & FRONTSTAGE_ENABLESHIFTSELECTION);
}

inline BOOL CFrontstageItemView::IsDragAndDropEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLEDRAGANDDROP)==0) || IsFocusItemEnabled());

	return (m_Flags & FRONTSTAGE_ENABLEDRAGANDDROP);
}

inline BOOL CFrontstageItemView::IsLabelEditEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLELABELEDIT)==0) || IsFocusItemEnabled());

	return (m_Flags & FRONTSTAGE_ENABLELABELEDIT);
}

inline BOOL CFrontstageItemView::IsEditOnHoverEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_ENABLEEDITONHOVER)==0) || IsLabelEditEnabled());

	return (m_Flags & FRONTSTAGE_ENABLEEDITONHOVER);
}

inline BOOL CFrontstageItemView::IsHideSelectionOnEditEnabled() const
{
	ASSERT(((m_Flags & FRONTSTAGE_HIDESELECTIONONEDIT)==0) || IsLabelEditEnabled());

	return (m_Flags & FRONTSTAGE_HIDESELECTIONONEDIT);
}

inline INT CFrontstageItemView::GetGutterForMargin(INT Margin)
{
	return (Margin>=BACKSTAGEBORDER) ? ITEMVIEWMARGINLARGE : (Margin>=ITEMCELLPADDINGY) ? ITEMVIEWMARGIN : ITEMCELLPADDINGY;
}

inline void CFrontstageItemView::SetItemHeight(const CIcons& Icons, INT Rows, INT Padding)
{
	ASSERT(Rows>=1);
	ASSERT(Padding>=0);

	SetItemHeight(Icons.GetIconSize(), Rows, Padding);
}

inline void CFrontstageItemView::ResetItemCategories()
{
	for (UINT a=0; a<m_ItemCategories.m_ItemCount; a++)
		ZeroMemory(&m_ItemCategories[a].Rect, sizeof(RECT));
}

inline UINT CFrontstageItemView::GetItemCount() const
{
	return m_ItemCount;
}

inline ItemData* CFrontstageItemView::GetItemData(INT Index) const
{
	ASSERT(m_pItemData);
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	return (ItemData*)(m_pItemData+Index*m_szData);
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

inline void CFrontstageItemView::SelectNone()
{
	SendMessage(WM_COMMAND, IDM_ITEMVIEW_SELECTNONE);
}

inline void CFrontstageItemView::ResetDragLocation()
{
	m_DragPos.x = m_DragPos.y = -1;
}

inline BOOL CFrontstageItemView::IsDragLocationValid() const
{
	return (m_DragPos.x!=-1) && (m_DragPos.y!=-1);
}

inline BOOL CFrontstageItemView::IsEditItem(INT Index) const
{
	return IsEditing() && (Index==m_EditItem) && IsHideSelectionOnEditEnabled();
}

inline BOOL CFrontstageItemView::DrawHover(INT Index) const
{
	return IsEditItem(Index) ? FALSE : Index==m_HoverItem;
}

inline BOOL CFrontstageItemView::DrawSelection(INT Index) const
{
	return IsEditItem(Index) ? FALSE : IsSelectionEnabled() ? IsItemSelected(Index) : (m_FocusItem==Index) && m_FocusItemSelected;
}

inline COLORREF CFrontstageItemView::SetLightTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return dc.SetTextColor(GetLightTextColor(dc, Index, Themed));
}

inline COLORREF CFrontstageItemView::SetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return dc.SetTextColor(GetDarkTextColor(dc, Index, Themed));
}
