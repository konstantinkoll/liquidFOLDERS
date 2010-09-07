
// CExplorerTree: Schnittstelle der Klasse CExplorerTree
//

#pragma once
#include "LFApplication.h"
#include "LFTooltip.h"


// CExplorerTree
//

class AFX_EXT_CLASS CExplorerTree : public CTreeCtrl
{
public:
	CExplorerTree();

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL OnlyFilesystem=TRUE, CString RootPath=_T(""));
	LPITEMIDLIST GetSelectedPIDL();

protected:
	LFApplication* p_App;
	LFTooltip m_TooltipCtrl;
	IContextMenu2* m_pContextMenu2;
	BOOL m_OnlyFilesystem;
	BOOL m_Hover;
	HTREEITEM m_HoverItem;
	CString m_RootPath;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CString OnGetItemText(LPAFX_SHELLITEMINFO pItem);
	int OnGetItemIcon(LPAFX_SHELLITEMINFO pItem, BOOL bSelected);
	void PopulateTree();
	BOOL GetChildItems(HTREEITEM hParentItem);
	void EnumObjects(HTREEITEM hParentItem, IShellFolder* pParentFolder, LPITEMIDLIST pidlParent);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
