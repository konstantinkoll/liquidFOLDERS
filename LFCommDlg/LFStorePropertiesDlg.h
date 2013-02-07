
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
	CPropertyPage* m_pPages[3];
	UINT m_PageCount;
	LFStoreDescriptor m_Store;
	BOOL m_StoreValid;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	_GUID m_Key;

	void UpdateStore(UINT Message, WPARAM wParam, LPARAM lParam);
};
