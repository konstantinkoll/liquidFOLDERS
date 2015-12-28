
// LFEditFilterDlg.h: Schnittstelle der Klasse LFEditFilterDlg
//

#pragma once
#include "CExplorerList.h"
#include "LFDialog.h"


// CConditionList
//

class CConditionList : public CExplorerList
{
public:
	CConditionList();

	void InsertItem(LFFilterCondition* c);
	void SetItem(INT nItem, LFFilterCondition* c);

protected:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	static void ConditionToItem(LFFilterCondition* c, LVITEM& lvi);
	void FinishItem(INT Index, LFFilterCondition* c);

	CString m_Compare[LFFilterCompareCount];
};


// LFEditFilterDlg
//

class LFEditFilterDlg : public LFDialog
{
public:
	LFEditFilterDlg(CWnd* pParentWnd=NULL, CHAR* StoreID=NULL, LFFilter* pFilter=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFFilter* CreateFilter();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSave();

	afx_msg void OnAddCondition();
	afx_msg void OnEditCondition();
	afx_msg void OnDeleteCondition();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CHAR m_StoreID[LFKeySize];
	LFFilter* p_Filter;
	LFDynArray<LFFilterCondition, 4, 4> m_Conditions;

	CButton m_wndAllStores;
	CButton m_wndThisStore;
	CEdit m_wndSearchterm;
	CConditionList m_wndConditionList;
	CImageList m_AttributeIcons;
};
