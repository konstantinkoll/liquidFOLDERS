
// ChoosePropertyDlg.h: Schnittstelle der Klasse ChoosePropertyDlg
//

#pragma once
#include "LFCommDlg.h"
#include "Migrate.h"


// ChoosePropertyDlg
//

class ChoosePropertyDlg : public LFAttributeListDlg
{
public:
	ChoosePropertyDlg(CWnd* pParent, INT Attr=-1);

	virtual void DoDataExchange(CDataExchange* pDX);

	INT m_Attr;

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnReset();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
