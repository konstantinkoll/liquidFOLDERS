
// CShellTree: Schnittstelle der Klasse CShellTree
//

#pragma once
#include "CFrontstageWnd.h"


// CShellTree
//

#define WM_SHELLCHANGE     WM_USER+5

struct ExplorerTreeItemData
{
	LPITEMIDLIST pidlFQ;
	LPITEMIDLIST pidlRel;
	SFGAOF dwAttributes;
};

class CShellTree : public CTreeCtrl
{
public:
	CShellTree();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	LPCITEMIDLIST GetSelectedPIDL() const;
	BOOL GetSelectedPath(LPWSTR Path) const;

protected:
	virtual LRESULT WindowProc(UINT Message, WPARAM wParam, LPARAM lParam);

	CString GetItemText(LPITEMIDLIST pidlFQ) const;
	CString GetItemText(ExplorerTreeItemData* pItem) const;
	INT GetItemIcon(LPITEMIDLIST pidlFQ, BOOL bSelected) const;
	INT GetItemIcon(ExplorerTreeItemData* pItem, BOOL bSelected) const;
	HTREEITEM InsertItem(LPITEMIDLIST pidlFQ, LPITEMIDLIST pidlRel, SFGAOF dwAttributes=SFGAO_HASSUBFOLDER, HTREEITEM hParent=TVI_ROOT);
	HTREEITEM InsertItem(LPCWSTR Path, HTREEITEM hParent=TVI_ROOT);
	BOOL GetChildItems(HTREEITEM hParentItem);
	void EnumObjects(HTREEITEM hParentItem, LPITEMIDLIST pidlParent);
	BOOL ChildrenContainPath(HTREEITEM hParentItem, LPCWSTR Path) const;
	BOOL DeletePath(LPCWSTR Path);
	BOOL AddPath(LPCWSTR Path, LPCWSTR Parent);
	void UpdateChildPIDLs(HTREEITEM hParentItem, LPITEMIDLIST pidlParent);
	void UpdatePath(LPCWSTR Path1, LPCWSTR Path2);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	DECLARE_TOOLTIP()

	IContextMenu2* m_pContextMenu2;

private:
	ULONG m_SHChangeNotifyRegister;
};

inline CString CShellTree::GetItemText(ExplorerTreeItemData* pItem) const
{
	ASSERT(pItem);

	return GetItemText(pItem->pidlFQ);
}

inline INT CShellTree::GetItemIcon(ExplorerTreeItemData* pItem, BOOL bSelected) const
{
	ASSERT(pItem);

	return GetItemIcon(pItem->pidlFQ, bSelected);
}
