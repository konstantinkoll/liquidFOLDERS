
#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CWorkflowWnd
//

class CWorkflowWnd : public LFCaptionBar
{
public:
	CWorkflowWnd();
	virtual ~CWorkflowWnd();

	CToolbarList m_wndList;

	void UpdateList();

	afx_msg void OnStoreNew();

protected:
	CImageList m_Icons;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemActivate(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
