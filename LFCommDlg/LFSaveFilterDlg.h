
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
	LFSaveFilterDlg(CWnd* pParentWnd=NULL, const CHAR* pStoreID=NULL, BOOL AllowChooseStore=FALSE, LPCWSTR FileName=NULL, LPCWSTR Comments=NULL);

	CHAR m_StoreID[LFKeySize];
	WCHAR m_FileName[256];
	WCHAR m_Comments[256];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnChooseStore();
	afx_msg void OnChange();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL m_AllowChooseStore;

private:
	CStorePanel m_wndStorePanel;
	CEdit m_wndEdit;
};
