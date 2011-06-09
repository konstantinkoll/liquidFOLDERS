
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

	virtual void SetOwner(CWnd* pOwnerWnd);
	virtual void AdjustLayout();

	void SetStoreID(CHAR* StoreID);

protected:
	UINT m_FontHeight;
	CHAR m_StoreID[LFKeySize];
	DynArray<LFFilterCondition> m_Conditions;
	CLabel m_wndLabel1;
	CButton m_wndAllStores;
	CButton m_wndThisStore;
	CLabel m_wndLabel2;
	CEdit m_wndSearchterm;
	CLabel m_wndLabel3;
	CButton m_wndAddCondition;
	CConditionList m_wndList;
	CBottomArea m_wndBottomArea;

	LFFilter* CreateFilter();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnSave();
	afx_msg void OnSearch();
	afx_msg void OnAddCondition();
	afx_msg void OnEditCondition();
	afx_msg void OnDeleteCondition();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
