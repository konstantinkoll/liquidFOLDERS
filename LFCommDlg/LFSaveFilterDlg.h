
// LFSaveFilterDlg.h: Schnittstelle der Klasse LFSaveFilterDlg
//

#pragma once
#include "CStorePanel.h"
#include "LFDialog.h"


// LFSaveFilterDlg
//

class LFSaveFilterDlg : public LFDialog
{
public:
	LFSaveFilterDlg(CWnd* pParentWnd=NULL, CHAR* StoreID=NULL, BOOL AllowChooseStore=FALSE, WCHAR* FileName=NULL, WCHAR* Comments=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	CHAR m_StoreID[LFKeySize];
	WCHAR m_FileName[256];
	WCHAR m_Comments[256];

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnChooseStore();
	afx_msg void OnChange();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL m_AllowChooseStore;

private:
	CStorePanel m_wndStorePanel;
	CEdit m_wndEdit;
};
