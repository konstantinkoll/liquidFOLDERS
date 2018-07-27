
// LFEditConditionDlg.h: Schnittstelle der Klasse LFEditConditionDlg
//

#pragma once
#include "CExplorerList.h"
#include "CPropertyEdit.h"
#include "LFAttributeListDlg.h"


// LFEditConditionDlg
//

class LFEditConditionDlg : public LFAttributeListDlg
{
public:
	LFEditConditionDlg(const STOREID& StoreID, CWnd* pParentWnd=NULL, LFFilterCondition* pCondition=NULL);

	LFFilterCondition m_Condition;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);
	virtual BOOL InitDialog();

	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	STOREID m_StoreID;
	CExplorerList m_wndAttribute;
	CComboBox m_wndCompare;
	CPropertyEdit m_wndEdit;
};
