
// OrganizeDlg.h: Schnittstelle der Klasse OrganizeDlg
//

#pragma once
#include "liquidFOLDERS.h"


// OrganizeDlg
//

class OrganizeDlg : public LFAttributeListDlg
{
public:
	OrganizeDlg(UINT Context, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	LFViewParameters* p_ViewParameters;
	UINT m_Context;
	CExplorerList m_wndSortAttribute;
	CComboBox m_wndSortDirection;
};
