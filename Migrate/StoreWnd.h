
#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CStoreBar
//

class CStoreToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList()
	{
		return FALSE;
	}
};


// CStoreWnd
//

class CStoreWnd : public CDockablePane
{
public:
	CStoreWnd();
	virtual ~CStoreWnd();

	void UpdateStores(BOOL FocusDefaultStore);

protected:
	LFSearchResult* result;
	CStoreToolBar m_wndToolBar;
	CImageList m_Icons;
	CPaneList m_wndList;
	BOOL m_Categories;

	void AddStoreItem(LFItemDescriptor* i);
	int GetSelectedItem();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT UpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnStoreNew();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreMakeHybrid();
	afx_msg void OnStoreProperties();
	afx_msg void OnStoreShowCategories();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
