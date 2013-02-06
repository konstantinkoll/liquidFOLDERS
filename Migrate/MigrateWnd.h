
// MigrateWnd.h: Headerdatei
//

#pragma once
#include "CMainView.h"
#include "CPIDLSelector.h"


// CMigrateWnd

class CMigrateWnd : public CGlassWindow
{
public:
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create();

protected:
	CPIDLSelector m_wndFolder;
	CStoreSelector m_wndStore;
	CMainView m_wndMainView;
	CBottomArea m_wndBottomArea;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg void OnSelectRoot();
	afx_msg void OnMigrate();
	afx_msg void OnRootChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRootUpdate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
