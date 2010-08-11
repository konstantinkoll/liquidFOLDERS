
// MigrateWnd.h: Headerdatei
//

#pragma once
#include "CMainView.h"
#include "CBottomArea.h"


// CMigrateWnd

class CMigrateWnd : public CGlasWindow
{
public:
	CMigrateWnd();
	~CMigrateWnd();

	virtual void AdjustLayout();

	BOOL Create();

protected:
	CMainView m_wndMainView;
	CBottomArea m_wndBottomArea;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAbout();
	afx_msg void OnNewStoreManager();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	HICON m_hIcon;
};
