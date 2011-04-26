
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
	EditConditionDlg(CWnd* pParent);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
