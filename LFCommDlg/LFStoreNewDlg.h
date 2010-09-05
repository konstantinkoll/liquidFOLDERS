
// LFStoreNewDlg.h: Schnittstelle der Klasse LFStoreNew
//

#pragma once
#include "liquidFOLDERS.h"
#include "CIconCtrl.h"


// LFStoreNewDlg
//

class AFX_EXT_CLASS LFStoreNewDlg : public CDialog
{
public:
	LFStoreNewDlg(CWnd* pParentWnd, UINT nIDTemplate, char Drive, LFStoreDescriptor* _store);

	virtual void DoDataExchange(CDataExchange* pDX);

	bool makeDefault;

protected:
	void PopulateListCtrl();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void SetInternalIcon();
	afx_msg void SetOptions();
	afx_msg LRESULT OnMediaChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor* store;

	CImageList m_Icons;
	CIconCtrl m_IconInternal;
	CIconCtrl m_IconHybrid;
	CIconCtrl m_IconExternal;
	ULONG m_ulSHChangeNotifyRegister;
	UINT m_nIDTemplate;
	char m_Drive;
};
