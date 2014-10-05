
// MigrationWnd.h: Headerdatei
//

#pragma once
#include "CMigrationView.h"
#include "CPIDLSelector.h"


// CMigrationWnd

class CMigrationWnd : public CGlassWindow
{
public:
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create(CHAR* Store);

protected:
	CHAR* p_Store;
	CPIDLSelector m_wndFolder;
	CStoreSelector m_wndStore;
	CMigrationView m_wndMigrationView;
	CFooterArea m_wndBottomArea;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnSelectRoot();
	afx_msg void OnMigrate();
	afx_msg void OnRootChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRootUpdate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
