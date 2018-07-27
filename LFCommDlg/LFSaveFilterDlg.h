
// LFSaveFilterDlg.h: Schnittstelle der Klasse LFSaveFilterDlg
//

#pragma once
#include "CItemPanel.h"
#include "LFDialog.h"


// LFSaveFilterDlg
//

class LFSaveFilterDlg : public LFDialog
{
public:
	LFSaveFilterDlg(const STOREID& StoreID, CWnd* pParentWnd=NULL, LPCWSTR FileName=NULL, LPCWSTR Comments=NULL);

	STOREID m_StoreID;
	WCHAR m_FileName[256];
	WCHAR m_Comments[256];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnChooseStore();
	afx_msg void OnChange();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL m_IsValidStore;

private:
	CItemPanel m_wndStorePanel;
	CEdit m_wndEdit;
};
