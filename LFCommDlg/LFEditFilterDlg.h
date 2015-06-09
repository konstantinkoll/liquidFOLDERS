
// LFEditFilterDlg.h: Schnittstelle der Klasse LFEditFilterDlg
//

#pragma once
#include "LFCommDlg.h"


// LFEditFilterDlg
//

class LFEditFilterDlg : public LFDialog
{
public:
	LFEditFilterDlg(CWnd* pParentWnd, CHAR* StoreID=NULL, LFFilter* pFilter=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CHAR m_StoreID[LFKeySize];
	LFFilter* p_Filter;
	LFDynArray<LFFilterCondition> m_Conditions;
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
