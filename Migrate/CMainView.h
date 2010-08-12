
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "LFCommDlg.h"
#include "CHeaderView.h"
#include "CTreeView.h"


// CMainView
//

class CMainView : public CWnd
{
public:
	CMainView();

	CString Root;

	void Create(CWnd* _pParentWnd, UINT nID);
	void ClearRoot();
	void SetRoot(CString _Root);

protected:
	CTaskbar m_wndTaskbar;
	CExplorerHeader m_wndExplorerHeader;
	CHeaderView m_wndHeader;
	CTreeView m_wndTree;
	BOOL IsRootSet;

	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnAbout();
	afx_msg void OnNewStoreManager();
	DECLARE_MESSAGE_MAP()
};
