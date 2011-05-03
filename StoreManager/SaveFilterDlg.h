
// SaveFilterDlg.h: Schnittstelle der Klasse SaveFilterDlg
//

#pragma once
#include "LFCommDlg.h"


// SaveFilterDlg
//

class SaveFilterDlg : public CDialog
{
public:
	SaveFilterDlg(CWnd* pParentWnd, CHAR* StoreID=NULL, WCHAR* FileName=NULL, BOOL AllowChooseStore=FALSE);

	virtual void DoDataExchange(CDataExchange* pDX);

	CHAR m_StoreID[LFKeySize];
	WCHAR m_FileName[256];

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
