
// CFrontstageItemView: Schnittstelle der Klasse CFrontstageItemView
//

#pragma once
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

#define LFITEMVIEWMARGIN      5

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
	CFrontstageItemView(SIZE_T DataSize=sizeof(ItemData), UINT Flags=FRONTSTAGE_ENABLESCROLLING, const CSize& szItemInflate=CSize(0, 0));
	BOOL Create(CWnd* pParentWnd, UINT nID, const CRect& rect=CRect(0, 0, 0, 0), UINT nClassStyle=0);
	INT GetSelectedItem() const;
	void SelectNone();

protected:
	virtual INT GetItemCategory(INT Index) const;
	virtual INT CompareItems(INT Index1, INT Index2) const;
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual COLORREF GetItemTextColor(INT Index) const;
	virtual BOOL IsItemSelected(INT Index) const;
	virtual void SelectItem(INT Index, BOOL Select=TRUE);
	virtual INT HandleNavigationKeys(UINT nChar, BOOL Control) const;
	virtual void FireFocusItem() const;
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
	void AddItemCategory(LPCWSTR Caption, LPCWSTR Hint=L"");
	void AllocItemData(UINT ItemCount);
	void SetItemCount(UINT ItemCount);
	void AddItem(LPCVOID pData);
	ItemData* GetItemData(INT Index) const;
	void FreeItemData(BOOL InternalCall);
	void ValidateAllItems();
	void SortItems();
	RECT GetItemRect(INT Index) const;
	INT GetFocusItem() const;
	void SetFocusItem(INT Index, UINT Mode=SETFOCUSITEM_MOVEDESELECT);
	BOOL HasItemsSelected() const;
	void ItemSelectionChanged(INT Index);
	void ItemSelectionChanged();
	void EnsureVisible(INT Index);
	void ResetDragLocation();
	BOOL IsDragLocationValid() const;
	void DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	void DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	COLORREF SetLightTextColor(CDC& dc, INT Index, BOOL Themed) const;
	COLORREF SetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const;
	void GetLayoutRect(CRect& rectLayout);
	void AdjustLayoutGrid(const CSize& szItem, const CSize& szGutter, BOOL FullWidth=FALSE, INT Margin=LFITEMVIEWMARGIN);
	void AdjustLayoutSingleColumnList();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnSelectToggle();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	INT m_LargeFontHeight;
	INT m_DefaultFontHeight;
	INT m_SmallFontHeight;

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
	void Swap(INT Index1, INT Index2);
	void Heap(INT Element, INT Count);

	SIZE_T m_DataSize;
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

	return ((ItemData*)(m_pItemData+Index*m_DataSize));
}

inline void CFrontstageItemView::Swap(INT Index1, INT Index2)
{
	ASSERT(Index1>=0);
	ASSERT(Index1>=0);
	ASSERT(Index1<m_ItemCount);
	ASSERT(Index2<m_ItemCount);

	LPBYTE Ptr1 = (LPBYTE)GetItemData(Index1);
	LPBYTE Ptr2 = (LPBYTE)GetItemData(Index2);

	for (SIZE_T sz=0; sz<m_DataSize; sz++)
	{
		const BYTE b = *Ptr1;
		*(Ptr1++) = *Ptr2;
		*(Ptr2++) = b;
	}
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

inline void CFrontstageItemView::AdjustLayoutSingleColumnList()
{
	AdjustLayoutGrid(CSize(0, m_ItemHeight), CSize(LFITEMVIEWMARGIN, -1), TRUE);
}
