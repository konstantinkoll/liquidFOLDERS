
#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CFilterToolBar
//

class CFilterToolBar : public CMFCToolBar
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


// CFilterWnd
//

class CFilterWnd : public CGlasPane
{
public:
	CFilterWnd();
	virtual ~CFilterWnd();

	virtual void AdjustLayout();

	void UpdateList();

protected:
	CFilterToolBar m_wndToolBar;
	CLabel m_wndLabel1;
	CEdit m_wndFreetext;
	CLabel m_wndLabel2;
	CComboBox m_wndStoreCombo;
	CButton m_wndStartSearch;
	CButton m_wndAddCondition;
	CLabel m_wndLabel3;
	CImageList* m_Icons;
	CExplorerList m_wndList;

	void AddConditionItem(BOOL focus=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
