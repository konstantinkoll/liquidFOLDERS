
// LFDeleteStoreDlg.h: Schnittstelle der Klasse LFDeleteStoreDlg
//

#pragma once
#include "LFCore.h"
#include "LFDialog.h"


// LFDeleteStoreDlg
//

class LFDeleteStoreDlg : public LFDialog
{
public:
	LFDeleteStoreDlg(const CHAR* pStoreID, CWnd* pParentWnd=NULL);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void SetOkButton();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_StoreID[LFKeySize];
	LFStoreDescriptor m_Store;
};
