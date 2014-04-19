
// ChoosePropertyDlg.h: Schnittstelle der Klasse ChoosePropertyDlg
//

#pragma once
#include "LFCommDlg.h"
#include "StoreManager.h"


// ChoosePropertyDlg
//

class ChoosePropertyDlg : public LFAttributeListDlg
{
public:
	ChoosePropertyDlg(CWnd* pParentWnd, INT Attr=-1);

	virtual void DoDataExchange(CDataExchange* pDX);

	INT m_Attr;

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnReset();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};