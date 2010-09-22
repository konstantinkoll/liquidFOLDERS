
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "LFCommDlg.h"
#include "CTreeView.h"


// CMainView
//

class CMainView : public CWnd
{
public:
	CMainView();

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	int Create(CWnd* _pParentWnd, UINT nID);
	void ClearRoot();
	void SetRoot(LPITEMIDLIST pidl, BOOL Update);

protected:
	CTaskbar m_wndTaskbar;
	CExplorerHeader m_wndExplorerHeader;
	CTreeView m_wndTree;
	BOOL m_IsRootSet;
	BOOL m_SelectedHasChildren;
	BOOL m_SelectedCanRename;
	BOOL m_SelectedCanDelete;

	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSelectRoot();
	afx_msg void OnUpdateTaskbar(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()
};
