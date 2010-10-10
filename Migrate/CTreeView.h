
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
#define CF_CANEXPAND        2
#define CF_CANCOLLAPSE      4
#define CF_HASCHILDREN      8
#define CF_HASSIBLINGS      16
#define CF_ISSIBLING        32
#define CF_CANRENAME        64
#define CF_CANDELETE        128
#define CF_HASPROPSHEET     256

struct ItemData
{
	LPITEMIDLIST pidlFQ;
	LPITEMIDLIST pidlRel;
	wchar_t Name[256];
	int IconIDNormal;
	int IconIDSelected;
	int Width;
	wchar_t Path[MAX_PATH];
};

struct Cell
{
	ItemData* pItem;
	UINT Flags;
};

struct tagTreeView
{
	NMHDR hdr;
	Cell* pCell;
};

class CTreeView : public CWnd
{
public:
	CTreeView();
	~CTreeView();

	BOOL Create(CWnd* _pParentWnd, UINT nID);
	void AdjustLayout();
	void ClearRoot();
	void SetRoot(LPITEMIDLIST pidl, BOOL Update, BOOL ExpandAll);
	void SetBranchCheck(BOOL Check, CPoint item=CPoint(-1, -1));
	void ExpandFolder(CPoint item=CPoint(-1, -1), BOOL ExpandAll=FALSE);
	void OpenFolder(CPoint item=CPoint(-1, -1));
	void DeleteFolder(CPoint item=CPoint(-1, -1));
	void ShowProperties(CPoint item=CPoint(-1, -1));
	void AutosizeColumns();
	void EditLabel(CPoint item=CPoint(-1, -1));

protected:
	Cell* m_Tree;
	CEdit* p_Edit;
	CTooltipHeader m_wndHeader;
	int m_ColumnWidth[MaxColumns];
	int m_ColumnMapping[MaxColumns];
	HTHEME hThemeList;
	HTHEME hThemeButton;
	HTHEME hThemeTree;
	LFTooltip m_TooltipCtrl;
	IContextMenu2* m_pContextMenu2;
	UINT m_HeaderHeight;
	UINT m_Allocated;
	UINT m_Rows;
	UINT m_Cols;
	UINT m_RowHeight;
	CSize m_IconSize;
	CSize m_CheckboxSize;
	CSize m_GlyphSize;
	CPoint m_SelectedItem;
	CPoint m_HotItem;
	CPoint m_HotExpando;
	CPoint m_EditLabel;
	BOOL m_CheckboxHot;
	BOOL m_CheckboxPressed;
	BOOL m_Hover;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL InsertRow(UINT row);
	void RemoveRows(UINT first, UINT last);
	void UpdateChildPIDLs(UINT row, UINT col);
	void SetItem(UINT row, UINT col, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags);
	void RemoveItem(UINT row, UINT col);
	UINT EnumObjects(UINT row, UINT col, BOOL ExpandAll, BOOL FirstInstance=TRUE);
	void Expand(UINT row, UINT col, BOOL ExpandAll, BOOL AutosizeHeader=TRUE);
	void Collapse(UINT row, UINT col);
	void FreeItem(Cell* cell);
	void FreeTree();
	BOOL HitTest(CPoint point, CPoint* item, BOOL* cbhot, CPoint* exphot);
	void InvalidateItem(CPoint item);
	void TrackMenu(UINT nID, CPoint point, int col=-1);
	void SelectItem(CPoint Item);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroyEdit();
	afx_msg LRESULT OnChooseProperty(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	IShellFolder* pDesktop;
	CImageList m_DefaultGlyphs;

	void SetWidgetSize();
	UINT GetChildRect(CPoint item);
	void NotifyOwner();
	BOOL ExecuteContextMenu(CPoint& item, LPCSTR verb);
	CString GetColumnCaption(UINT col);
	void UpdateColumnCaption(UINT col);
	void AutosizeColumn(UINT col, BOOL OnlyEnlarge=FALSE);
	void ExpandColumn(UINT col);
	void DestroyEdit(BOOL Accept=FALSE);
};
