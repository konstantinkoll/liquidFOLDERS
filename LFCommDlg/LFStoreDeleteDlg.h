
// LFStoreDeleteDlg.h: Schnittstelle der Klasse LFStoreDeleteDlg
//

#pragma once
#include "LFCommDlg.h"


// LFStoreDeleteDlg
//

class LFStoreDeleteDlg : public LFDialog
{
public:
	LFStoreDeleteDlg(CHAR* StoreID, CWnd* pParentWnd=NULL);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void SetOkButton();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_Key[LFKeySize];
	LFStoreDescriptor m_Store;
};
