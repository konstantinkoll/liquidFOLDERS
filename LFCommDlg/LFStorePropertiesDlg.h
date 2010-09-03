#pragma once
#include <shlobj.h>
#include "liquidFOLDERS.h"

class AFX_EXT_CLASS LFStorePropertiesDlg : public CDialog
{
public:
	LFStorePropertiesDlg(char* _StoreID, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT UpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor store;
	_GUID key;
};
