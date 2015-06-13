
// ChooseDetailsDlg.h: Schnittstelle der Klasse ChooseDetailsDlg
//

#pragma once
#include "LFCommDlg.h"
#include "liquidFOLDERS.h"


// ChooseDetailsDlg
//

class ChooseDetailsDlg : public LFAttributeListDlg
{
public:
	ChooseDetailsDlg(UINT Context, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	DECLARE_MESSAGE_MAP()

	CListCtrl m_ShowAttributes;
	LFViewParameters* p_View;
	UINT m_Context;

private:
	void SwapItems(INT FocusItem, INT NewPos);
};
