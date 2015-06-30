
// LFEditConditionDlg.h: Schnittstelle der Klasse LFEditConditionDlg
//

#pragma once
#include "CPropertyEdit.h"
#include "LFAttributeListDlg.h"


// LFEditConditionDlg
//

class LFEditConditionDlg : public LFAttributeListDlg
{
public:
	LFEditConditionDlg(CWnd* pParentWnd=NULL, CHAR* StoreID=NULL, LFFilterCondition* pCondition=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFFilterCondition m_Condition;

protected:
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_StoreID[LFKeySize];
	CListCtrl m_wndAttribute;
	CComboBox m_wndCompare;
	CPropertyEdit m_wndEdit;
};
