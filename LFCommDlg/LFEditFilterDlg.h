
// LFEditFilterDlg.h: Schnittstelle der Klasse LFEditFilterDlg
//

#pragma once
#include "CFrontstageItemView.h"
#include "LFDialog.h"


// CConditionList
//

typedef LFDynArray<LFFilterCondition, 4, 4> ConditionArray;

class CConditionList sealed : public CFrontstageItemView
{
public:
	CConditionList();

	void SetConditions(const ConditionArray& Conditions);

protected:
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);
	virtual void AdjustLayout();
	virtual void FireSelectedItem() const;
	virtual void DeleteSelectedItem() const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	const ConditionArray* p_Conditions;

private:
	static CString m_strCompare[LFFilterCompareCount];
};


// LFEditFilterDlg
//

class LFEditFilterDlg : public LFDialog
{
public:
	LFEditFilterDlg(const STOREID& StoreID, CWnd* pParentWnd=NULL, LFFilter* pFilter=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	INT GetSelectedCondition() const;
	STOREID GetSelectedStore() const;
	LFFilter* CreateFilter() const;

	afx_msg void OnSave();

	afx_msg void OnAddCondition();
	afx_msg void OnEditCondition();
	afx_msg void OnDeleteCondition();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	STOREID m_StoreID;
	LFFilter* p_Filter;
	ConditionArray m_Conditions;

	CButton m_wndAllStores;
	CButton m_wndThisStore;
	CEdit m_wndSearchTerm;
	CConditionList m_wndConditionList;
};

inline INT LFEditFilterDlg::GetSelectedCondition() const
{
	return m_wndConditionList.GetSelectedItem();
}

inline STOREID LFEditFilterDlg::GetSelectedStore() const
{
	return m_wndAllStores.GetCheck() ? DEFAULTSTOREID() : m_StoreID;
}
