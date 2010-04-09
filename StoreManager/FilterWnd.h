
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

class CFilterWnd : public CDockablePane
{
public:
	CFilterWnd();
	virtual ~CFilterWnd();

	void UpdateList();

protected:
	CFilterToolBar m_wndToolBar;
	CPaneText m_wndText1;
	CEdit m_wndFreetext;
	CPaneText m_wndText2;
	CComboBox m_wndStoreCombo;
	CButton m_wndStartSearch;
	CButton m_wndAddCondition;
	CPaneText m_wndText3;
	CImageList* m_Icons;
	CPaneList m_wndList;

	void AddConditionItem(BOOL focus=FALSE);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnGotoHistory();
	afx_msg void OnNotifyGotoHistory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
