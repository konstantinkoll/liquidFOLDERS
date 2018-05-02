
// LFEditFilterDlg.h: Schnittstelle der Klasse LFEditFilterDlg
//

#pragma once
#include "CFrontstageItemView.h"
#include "LFDialog.h"


// CConditionList
//
typedef LFDynArray<LFFilterCondition, 4, 4> ConditionArray;

class CConditionList : public CFrontstageItemView
{
public:
	CConditionList();

	virtual void PreSubclassWindow();
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	void SetConditions(const ConditionArray& Conditions);

protected:
	virtual void AdjustLayout();
	virtual void FireSelectedItem() const;
	virtual void DeleteSelectedItem() const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	const ConditionArray* p_Conditions;
	INT m_IconSize;

private:
	CString m_Compare[LFFilterCompareCount];
};


// LFEditFilterDlg
//

class LFEditFilterDlg : public LFDialog
{
public:
	LFEditFilterDlg(CWnd* pParentWnd=NULL, const LPCSTR StoreID=NULL, LFFilter* pFilter=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	INT GetSelectedCondition() const;
	LFFilter* CreateFilter() const;

	afx_msg void OnSave();

	afx_msg void OnAddCondition();
	afx_msg void OnEditCondition();
	afx_msg void OnDeleteCondition();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CHAR m_StoreID[LFKeySize];
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
