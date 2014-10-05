
// SaveFilterDlg.h: Schnittstelle der Klasse SaveFilterDlg
//

#pragma once
#include "CStorePanel.h"


// SaveFilterDlg
//

class SaveFilterDlg : public CDialog
{
public:
	SaveFilterDlg(CWnd* pParentWnd, CHAR* StoreID=NULL, BOOL AllowChooseStore=FALSE, WCHAR* FileName=NULL, WCHAR* Comments=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	CHAR m_StoreID[LFKeySize];
	WCHAR m_FileName[256];
	WCHAR m_Comments[256];

protected:
	BOOL m_AllowChooseStore;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnChooseStore();
	afx_msg void OnChange();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CStorePanel m_wndStorePanel;
	CEdit m_wndEdit;

	void CheckOK();
};
