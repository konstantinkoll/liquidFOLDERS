
// LFStorePropertiesDlg.h: Schnittstelle der Klasse LFStorePropertiesDlg
//

#pragma once
#include "CIconCtrl.h"
#include "LFCore.h"
#include "LFTabbedDialog.h"


// LFStorePropertiesDlg
//

class LFStorePropertiesDlg : public LFTabbedDialog
{
public:
	LFStorePropertiesDlg(const LPCSTR pStoreID, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitSidebar(LPSIZE pszTabArea);
	virtual BOOL InitDialog();

	afx_msg void OnRunMaintenance();
	afx_msg void OnRunSynchronize();
	afx_msg void OnRunBackup();

	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	LFStoreDescriptor m_Store;
	UINT m_StoreIcon;
	UINT m_StoreType;
	BOOL m_StoreValid;
	static UINT m_LastTab;

	CIconCtrl m_wndStoreIcon;
	CEdit m_wndStoreName;
	CEdit m_wndStoreComment;
	CButton m_wndMakeDefault;
	CButton m_wndMakeSearchable;

	CIconCtrl m_wndSynchronizeIcon;
	CIconCtrl m_wndMaintenanceIcon;
	CIconCtrl m_wndBackupIcon;

private:
	_GUID m_StoreUniqueID;
	CString m_MaskMaintenance;
	CString m_MaskSynchronized;
};
