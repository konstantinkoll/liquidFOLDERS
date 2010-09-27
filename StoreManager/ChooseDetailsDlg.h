
// ChooseDetailsDlg.h: Schnittstelle der Klasse ChooseDetailsDlg
//

#pragma once
#include "LFCommDlg.h"
#include "StoreManager.h"


// ChooseDetailsDlg
//

class ChooseDetailsDlg : public LFAttributeListDlg
{
public:
	ChooseDetailsDlg(CWnd* pParentWnd, LFViewParameters* View, int Context, UINT nIDTemplate=IDD_CHOOSEDETAILS);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	CListCtrl m_ShowAttributes;
	LFViewParameters* p_View;
	UINT m_Context;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	DECLARE_MESSAGE_MAP()

private:
	void SwapItems(int FocusItem, int NewPos);

	UINT m_Template;
};
