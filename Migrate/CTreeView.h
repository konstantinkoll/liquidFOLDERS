
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

	BOOL Create(CWnd* _pParentWnd, UINT nID);
	void AdjustLayout();
	void ClearRoot();
	void SetRoot(LPITEMIDLIST pidl, BOOL Update);

protected:
	Cell* m_Tree;
	CTooltipHeader m_wndHeader;

	BOOL InsertRow(UINT Row);
	//BOOL RemoveRow(UINT Row);
	void SetItem(Cell* cell, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ);
	void FreeItem(Cell* cell);
	void FreeTree();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	UINT m_HeaderHeight;
	UINT m_Allocated;
	UINT m_Rows;
	UINT m_Cols;
};
