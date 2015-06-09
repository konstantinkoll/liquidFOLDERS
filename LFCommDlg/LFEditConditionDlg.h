
// LFEditConditionDlg.h: Schnittstelle der Klasse LFEditConditionDlg
//

#pragma once
#include "LFCommDlg.h"


// LFEditConditionDlg
//

class LFEditConditionDlg : public LFAttributeListDlg
{
public:
	LFEditConditionDlg(CWnd* pParentWnd, CHAR* StoreID=NULL, LFFilterCondition* pCondition=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFFilterCondition m_Condition;

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_StoreID[LFKeySize];
	CListCtrl m_wndAttribute;
	CComboBox m_wndCompare;
	CPropertyEdit m_wndEdit;
};
