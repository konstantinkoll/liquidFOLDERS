
// CExplorerTree: Schnittstelle der Klasse CExplorerTree
//

#pragma once
#include "LFApplication.h"


// CExplorerTree
//

class AFX_EXT_CLASS CExplorerTree : public CTreeCtrl
{
public:
	CExplorerTree();

	BOOL Create(CWnd* pParentWnd, UINT nID);

protected:
	CString OnGetItemText(LPAFX_SHELLITEMINFO pItem);
	int OnGetItemIcon(LPAFX_SHELLITEMINFO pItem, BOOL bSelected);
	void PopulateTree();
	BOOL GetChildItems(HTREEITEM hParentItem);
	void EnumObjects(HTREEITEM hParentItem, IShellFolder* pParentFolder, LPITEMIDLIST pidlParent);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	LFApplication* p_App;
	IContextMenu2* m_pContextMenu2;
};
