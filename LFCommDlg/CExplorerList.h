
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "CTooltipHeader.h"


// CExplorerList
//

class CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void AddCategory(INT ID, LPCWSTR Name, LPCWSTR Hint=L"", BOOL Collapsible=FALSE);
	void AddColumn(INT ID, LPCWSTR Name=L"", INT Width=100, BOOL Right=FALSE);
	void SetMenus(UINT ItemMenuID=0, BOOL HighlightFirst=FALSE, UINT BackgroundMenuID=0);
	void SetItemsPerRow(INT ItemsPerRow, INT ColumnsPerTile=3);

protected:
	virtual void Init();

	void SetWidgetSize();
	void AdjustLayout(INT ListWidth);
	void DrawItem(INT nID, CDC* pDC);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	INT m_ItemsPerRow;
	INT m_ColumnsPerTile;
	BOOL m_Hover;
	INT m_HoverItem;
	INT m_TooltipItem;
	HTHEME m_hThemeButton;
	CSize m_CheckboxSize;

private:
	void DrawIcon(CDC* pDC, CRect& rect, LVITEM& Item, UINT State);

	CTooltipHeader m_wndHeader;

	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;

	INT m_ColumnCount;
	LVCOLUMN m_Columns[16];
	CImageList* p_ImageList;
	INT m_IconSize;
	UINT m_View;
};
