
// MigrateWnd.h: Headerdatei
//

#pragma once
#include "CMainView.h"
#include "CPIDLSelector.h"
#include "LFCommDlg.h"


// CMigrateWnd

class CMigrateWnd : public CGlasWindow
{
public:
	CMigrateWnd();
	~CMigrateWnd();

	virtual void AdjustLayout();

	BOOL Create();

protected:
	CPIDLSelector m_wndFolder;
	CDropdownSelector m_wndStore;
	CMainView m_wndMainView;
	CBottomArea m_wndBottomArea;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnIdleUpdateCmdUI();
//	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	HICON m_hIcon;
};
