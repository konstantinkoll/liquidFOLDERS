#pragma once
#include <shlobj.h>
#include "liquidFOLDERS.h"

class AFX_EXT_CLASS LFStorePropertiesDlg : public CDialog
{
public:
	LFStorePropertiesDlg(CWnd* pParentWnd, char* _StoreID);
	virtual ~LFStorePropertiesDlg();

	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT UpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor store;
	_GUID key;
};
