
#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CFilterWnd
//

class CFilterWnd : public CGlasPane
{
public:
	CFilterWnd();
	~CFilterWnd();

	virtual void AdjustLayout();

protected:
	CLabel m_wndLabel1;
	CEdit m_wndFreetext;
	CLabel m_wndLabel2;
	CComboBox m_wndStoreCombo;
	CButton m_wndStartSearch;
	CButton m_wndAddCondition;
	CLabel m_wndLabel3;
	CConditionList m_wndList;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
