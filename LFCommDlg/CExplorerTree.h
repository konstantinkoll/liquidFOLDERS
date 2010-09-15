
// CExplorerTree: Schnittstelle der Klasse CExplorerTree
//

#pragma once
#include "LFApplication.h"
#include "LFTooltip.h"


// CExplorerTree
//

#define CETR_Desktop            _T("")
#define CETR_InternalDrives     _T("::DRVINTERNAL")
#define CETR_ExternalDrives     _T("::DRVEXTERNAL")

#define WM_SHELLCHANGE         WM_USER+2

class AFX_EXT_CLASS CExplorerTree : public CTreeCtrl
{
public:
	CExplorerTree();

	virtual void PreSubclassWindow();

	LPITEMIDLIST GetSelectedPIDL();
	BOOL GetSelectedPathA(LPSTR Path);
	BOOL GetSelectedPathW(LPWSTR Path);
	void PopulateTree();
	void SetRootPath(CString RootPath);
	void SetOnlyFilesystem(BOOL OnlyFilesystem);

protected:
	LFApplication* p_App;
	LFTooltip m_TooltipCtrl;
	IContextMenu2* m_pContextMenu2;
	BOOL m_OnlyFilesystem;
	BOOL m_Hover;
	BOOL m_ExplorerStyle;
	HTREEITEM m_HoverItem;
	CString m_RootPath;

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CString OnGetItemText(LPAFX_SHELLITEMINFO pItem);
	int OnGetItemIcon(LPAFX_SHELLITEMINFO pItem, BOOL bSelected);
	HTREEITEM InsertItem(IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, HTREEITEM hParent=TVI_ROOT, BOOL children=TRUE, LPITEMIDLIST pidlFQ=NULL);
	HTREEITEM InsertItem(wchar_t* Path, HTREEITEM hParent=TVI_ROOT);
	BOOL GetChildItems(HTREEITEM hParentItem);
	void EnumObjects(HTREEITEM hParentItem, IShellFolder* pParentFolder, LPITEMIDLIST pidlParent);
	BOOL ChildrenContainPath(HTREEITEM hParentItem, LPWSTR Path);
	BOOL DeletePath(LPWSTR Path);
	BOOL AddPath(LPWSTR Path, LPWSTR Parent);
	void UpdatePath(LPWSTR Path1, LPWSTR Path2, IShellFolder* pDesktop);

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	ULONG m_ulSHChangeNotifyRegister;
};
