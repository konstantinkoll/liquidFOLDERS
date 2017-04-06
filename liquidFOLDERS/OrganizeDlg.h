
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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void TestAttribute(UINT Attr, BOOL& Add, BOOL& Check);
	virtual BOOL InitDialog();

	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	UINT m_Context;
	LFContextViewSettings* p_ContextViewSettings;

	CExplorerList m_wndSortAttribute;
	CComboBox m_wndSortDirection;
};
