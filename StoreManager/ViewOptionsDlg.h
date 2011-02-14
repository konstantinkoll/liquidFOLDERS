
// ViewOptionsDlg.h: Schnittstelle der Klasse ViewOptionsDlg
//

#pragma once
#include "ChooseDetailsDlg.h"
#include "StoreManager.h"


// ViewOptionsDlg
//

class ViewOptionsDlg : public ChooseDetailsDlg
{
public:
	ViewOptionsDlg(CWnd* pParentWnd, UINT Context);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CImageListTransparent m_ViewIcons;
	CExplorerList m_wndViewList;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnViewModeChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
