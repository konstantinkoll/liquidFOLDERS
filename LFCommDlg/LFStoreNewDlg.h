#pragma once
#include <shlobj.h>
#include "liquidFOLDERS.h"

class AFX_EXT_CLASS LFStoreNewDlg : public CDialog
{
public:
	LFStoreNewDlg(CWnd* pParentWnd, UINT nIDTemplate, char Drive, LFStoreDescriptor* _store);
	virtual ~LFStoreNewDlg();

	virtual void DoDataExchange(CDataExchange* pDX);
	void PopulateListCtrl();

	bool makeDefault;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void SetInternalIcon();
	afx_msg void SetOptions();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnMediaChanged(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

private:
	CImageList icons;
	LFStoreDescriptor* store;
	ULONG m_ulSHChangeNotifyRegister;
	UINT m_nIDTemplate;
	char m_Drive;
};
