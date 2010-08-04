#pragma once
#include "LFDialog.h"

class AFX_EXT_CLASS LFStoreDeleteDlg : public LFDialog
{
public:
	LFStoreDeleteDlg(CWnd* pParentWnd, wchar_t* _StoreName);
	virtual ~LFStoreDeleteDlg();

	afx_msg BOOL OnInitDialog();
	afx_msg void SetOkButton();
	DECLARE_MESSAGE_MAP()

private:
	wchar_t* StoreName;
};
