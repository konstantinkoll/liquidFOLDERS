
// FileDropWnd.h: Headerdatei
//

#pragma once
#include "LFCommDlg.h"


// CMigrateWnd

class CMigrateWnd : public CGlasWindow
{
public:
	CMigrateWnd();
	~CMigrateWnd();

	BOOL Create();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnAbout();
	afx_msg void OnNewStoreManager();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	HICON m_hIcon;
};
