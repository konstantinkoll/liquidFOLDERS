
// CTreeView.h: Schnittstelle der Klasse CTreeView
//

#pragma once
#include "LFCommDlg.h"


// CTreeView
//

#define MaxColumns          16
#define FirstAlloc          1024
#define SubsequentAlloc     1024
#define MemoryAlignment     8

#define CF_CHECKED          1
#define CF_HASCHILDREN      2
#define CF_HASSIBLINGS      4
#define CF_ISSIBLING        8

struct ItemData
{
	LPITEMIDLIST pidlFQ;
	LPITEMIDLIST pidlRel;
	IShellFolder* pParentFolder;
	wchar_t Name[256];
	int IconIDNormal;
	int IconIDSelected;
	wchar_t Path[MAX_PATH];
};

struct Cell
{
	ItemData* pItem;
	UINT Flags;
};

class CTreeView : public CWnd
{
public:
	CTreeView();
	~CTreeView();

	BOOL Create(CWnd* _pParentWnd, UINT nID);
	void AdjustLayout();
	void ClearRoot();
	void SetRoot(LPITEMIDLIST pidl, BOOL Update);

protected:
	Cell* m_Tree;
	CTooltipHeader m_wndHeader;
	int m_ColumnWidth[MaxColumns];
	HTHEME hThemeList;
	HTHEME hThemeButton;
	UINT m_HeaderHeight;
	UINT m_Allocated;
	UINT m_Rows;
	UINT m_Cols;
	UINT m_RowHeight;
	int m_IconWidth;
	int m_IconHeight;
	CPoint m_Selected;
	CPoint m_Hot;
	BOOL m_CheckboxHot;
	BOOL m_CheckboxPressed;

	BOOL InsertRow(UINT Row);
	//BOOL RemoveRow(UINT Row);
	void SetItem(UINT row, UINT col, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags);
	UINT InsertItem(UINT row, UINT col, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags);
	void FreeItem(Cell* cell);
	void FreeTree();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	IShellFolder* pDesktop;
};
