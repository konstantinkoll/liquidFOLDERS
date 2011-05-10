
// EditConditionDlg.h: Schnittstelle der Klasse EditConditionDlg
//

#pragma once
#include "LFCommDlg.h"
#include "StoreManager.h"


// EditConditionDlg
//

class EditConditionDlg : public LFAttributeListDlg
{
public:
	EditConditionDlg(CWnd* pParent, LFFilterCondition* pCondition=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFFilterCondition m_Condition;

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

private:
	CListCtrl m_wndAttribute;
	CComboBox m_wndCompare;
	CPropertyEdit m_wndEdit;
};
