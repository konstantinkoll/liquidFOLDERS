
// LFStorePropertiesDlg.h: Schnittstelle der Klasse LFStorePropertiesDlg
//

#pragma once
#include "LFCore.h"
#include "LFCommDlg.h"


// LFStorePropertiesDlg
//

class AFX_EXT_CLASS LFStorePropertiesDlg : public CPropertySheet
{
public:
	LFStorePropertiesDlg(CHAR* StoreID, CWnd* pParent=NULL);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	CPropertyPage* m_Pages[3];
	UINT m_PageCount;
	LFStoreDescriptor m_Store;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};
