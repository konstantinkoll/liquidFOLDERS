
// EditFilterDlg.h: Schnittstelle der Klasse EditFilterDlg
//

#pragma once
#include "LFCommDlg.h"
#include "StoreManager.h"


// EditFilterDlg
//

class EditFilterDlg : public CDialog
{
public:
	EditFilterDlg(CWnd* pParentWnd, CHAR* StoreID=NULL, LFFilter* pFilter=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CHAR m_StoreID[LFKeySize];
	LFFilter* p_Filter;
	DynArray<LFFilterCondition> m_Conditions;
	CButton m_wndAllStores;
	CButton m_wndThisStore;
	CEdit m_wndSearchterm;
	CConditionList m_wndList;

	LFFilter* CreateFilter();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSave();

	afx_msg void OnAddCondition();
	afx_msg void OnEditCondition();
	afx_msg void OnDeleteCondition();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
