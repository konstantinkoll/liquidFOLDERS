
// CGenreList.h: Schnittstelle der Klasse CGenreList
//

#pragma once
#include "CFrontstageWnd.h"
#include "LFCore.h"


// Item Data

struct GenreCategoryData
{
	WCHAR Caption[256];
	INT IconID;
	RECT Rect;
};

struct GenreItemData
{
	LFMusicGenre* pMusicGenre;
	UINT Index;
	UINT FileCount;
	LPCWSTR pDescription;
	RECT Rect;
};


// CGenreList
//

class CGenreList : public CFrontstageWnd
{
public:
	CGenreList();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void AddCategory(LFMusicGenre* pMusicGenre);
	void AddItem(LFMusicGenre* pMusicGenre, INT Index, UINT FileCount, LPCWSTR pDescription);
	UINT GetSelectedGenre() const;
	void EnsureVisible(INT Index);
	void SelectGenre(UINT Genre);

protected:
	void ResetScrollbars();
	void AdjustScrollbars();
	void AdjustLayout();
	void SetFocusItem(INT FocusItem);
	RECT GetItemRect(INT Index) const;
	INT ItemAtPosition(CPoint point) const;
	void InvalidateItem(INT Index);
	void DrawItem(CDC& dc, CRect& rectItem, INT Index, BOOL Themed) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg UINT OnGetDlgCode();
	DECLARE_MESSAGE_MAP()

	LFDynArray<GenreCategoryData, 32, 32> m_Categories;
	LFDynArray<GenreItemData, 256, 16> m_Items;
	INT m_ScrollHeight;
	INT m_VScrollPos;
	INT m_VScrollMax;

private:
	INT m_CategoriesHeight;
	INT m_ItemsHeight;
	BOOL m_IsFirstItemInCategory;
	UINT m_ItemHeight;
	UINT m_CountWidth;
	INT m_FocusItem;
	INT m_HotItem;
	BOOL m_Hover;
};

inline UINT CGenreList::GetSelectedGenre() const
{
	return (m_FocusItem>=0) && (m_FocusItem<(INT)m_Items.m_ItemCount) ? m_Items[m_FocusItem].Index : 0;
}
