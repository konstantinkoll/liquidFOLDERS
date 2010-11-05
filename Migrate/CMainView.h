
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "LFCommDlg.h"
#include "CTreeView.h"
#include "CMigrationList.h"


// CMainView
//

class CMainView : public CWnd
{
public:
	CMainView();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	INT Create(CWnd* _pParentWnd, UINT nID);
	void ClearRoot();
	void SetRoot(LPITEMIDLIST pidl, BOOL Update, BOOL ExpandAll);
	void PopulateMigrationList(CMigrationList* ml, LFItemDescriptor* it);
	void UncheckMigrated(CReportList* rl);
	BOOL FoldersChecked();

protected:
	CTaskbar m_wndTaskbar;
	CExplorerHeader m_wndExplorerHeader;
	CTreeView m_wndTree;
	BOOL m_IsRootSet;
	BOOL m_SelectedCanExpand;
	BOOL m_SelectedHasPropSheet;
	BOOL m_SelectedCanRename;
	BOOL m_SelectedCanDelete;

	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnAutosizeAll();
	afx_msg void OnSelectRoot();
	afx_msg void OnExpand();
	afx_msg void OnOpen();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnProperties();
	afx_msg void OnUpdateTaskbar(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
