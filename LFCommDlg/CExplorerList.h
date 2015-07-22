
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "CTooltipHeader.h"
#include "LFTooltip.h"


// CExplorerList
//

#define REQUEST_TEXTCOLOR        1
#define REQUEST_TOOLTIP_DATA     2

struct NM_TEXTCOLOR
{
	NMHDR hdr;
	INT Item;
	COLORREF Color;
};

struct NM_TOOLTIPDATA
{
	NMHDR hdr;
	INT Item;
	BOOL Show;
	HICON hIcon;
	WCHAR Text[4096];
};

class CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, INT x, INT y, INT cx, INT cy, UINT nFlags);

	void AddCategory(INT ID, CString Name, CString Hint=_T(""), BOOL Collapsible=FALSE);
	void AddColumn(INT ID, LPWSTR Name, INT Width=100, BOOL Right=FALSE);
	void SetMenus(UINT ItemMenuID=0, BOOL HighlightFirst=FALSE, UINT BackgroundMenuID=0);
	void SetItemsPerRow(INT ItemsPerRow, INT ColumnsPerTile=3);

protected:
	virtual void Init();

	void AdjustLayout(INT ListWidth);
	void DrawItem(INT nID, CDC* pDC);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	LFTooltip m_TooltipCtrl;
	HTHEME hThemeList;
	INT m_ItemsPerRow;
	INT m_ColumnsPerTile;
	BOOL m_Hover;
	INT m_HoverItem;
	INT m_TooltipItem;

private:
	void DrawIcon(CDC* pDC, CRect& rect, LVITEM& Item, UINT State);

	CTooltipHeader m_wndHeader;

	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;

	UINT m_FontHeight;
	INT m_ColumnCount;
	LVCOLUMN m_Columns[16];
	CImageList* p_ImageList;
	INT m_IconSize;
	UINT m_View;
};
