
// CMigrationView.h: Schnittstelle der Klasse CMigrationView
//

#pragma once
#include "LFCommDlg.h"
#include "CTreeView.h"
#include "CMigrationList.h"


// CMigrationView
//

class CMigrationView : public CWnd
{
public:
	CMigrationView();

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

	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnSelectRoot();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
