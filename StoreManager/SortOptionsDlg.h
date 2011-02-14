
// SortOptionsDlg.h: Schnittstelle der Klasse SortOptionsDlg
//

#pragma once
#include "LFCommDlg.h"
#include "StoreManager.h"


// SortOptionsDlg
//

class SortOptionsDlg : public LFAttributeListDlg
{
public:
	SortOptionsDlg(CWnd* pParent, UINT Context);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void TestAttribute(UINT attr, BOOL& add, BOOL& check);

	LFViewParameters* p_View;
	UINT m_Context;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSetAttrGroupBox();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
