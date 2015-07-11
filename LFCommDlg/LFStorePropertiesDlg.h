
// LFStorePropertiesDlg.h: Schnittstelle der Klasse LFStorePropertiesDlg
//

#pragma once
#include "LFCore.h"


// LFStorePropertiesDlg
//

class LFStorePropertiesDlg : public CPropertySheet
{
public:
	LFStorePropertiesDlg(CHAR* StoreID, CWnd* pParentWnd=NULL);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStatisticsChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CPropertyPage* m_pPages[3];
	UINT m_PageCount;
	LFStoreDescriptor m_Store;
	BOOL m_StoreValid;

private:
	void UpdateStore(UINT Message, WPARAM wParam, LPARAM lParam);

	_GUID m_StoreID;
};
