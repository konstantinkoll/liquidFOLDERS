
// LFStorePropertiesDlg.h: Schnittstelle der Klasse LFStorePropertiesDlg
//

#pragma once
#include <shlobj.h>
#include "liquidFOLDERS.h"


// LFStorePropertiesDlg
//

class AFX_EXT_CLASS LFStorePropertiesDlg : public CDialog
{
public:
	LFStorePropertiesDlg(CHAR* _StoreID, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor store;
	_GUID key;
};
