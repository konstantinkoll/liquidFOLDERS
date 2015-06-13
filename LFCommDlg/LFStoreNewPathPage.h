
// LFStoreNewPathPage.h: Schnittstelle der Klasse LFStoreNewPathPage
//

#pragma once
#include "CExplorerTree.h"


// LFStoreNewPathPage
//

class LFStoreNewPathPage : public CPropertyPage
{
public:
	virtual void DoDataExchange(CDataExchange* pDX);

	CButton m_wndAutoPath;
	CExplorerTree m_wndPathTree;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnAutoPath();
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
