
// LFDeleteStoreDlg.h: Schnittstelle der Klasse LFDeleteStoreDlg
//

#pragma once
#include "LF.h"
#include "LFDialog.h"


// LFDeleteStoreDlg
//

class LFDeleteStoreDlg : public LFDialog
{
public:
	LFDeleteStoreDlg(CHAR* StoreID, CWnd* pParentWnd=NULL);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void SetOkButton();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_StoreID[LFKeySize];
	LFStoreDescriptor m_Store;
};
