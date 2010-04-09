
// MainFrm.h: Schnittstelle der Klasse CMainFrame
//

#pragma once
#include "PlacesWnd.h"
#include "StoreWnd.h"
#include "WorkflowWnd.h"
#include "liquidFOLDERS.h"
#include "CMainView.h"
#include "resource.h"

#define WM_USER_MEDIACHANGED                WM_USER+1


// CMainFrame
//

class CMainFrame : public CFrameWndEx
{
public:
	CMainFrame();
	virtual ~CMainFrame();

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	
	void ErrorBox(UINT res);
protected:
	CMFCRibbonBar m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;
	CMFCRibbonBaseElement* m_sbItemCount;
	CMFCRibbonBaseElement* m_sbHint;
	CPlacesWnd m_wndPlaces;
	CStoreWnd m_wndStores;
	CWorkflowWnd m_wndWorkflow;
	CMainView m_wndView;
	BOOL CaptionBarUsed;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnDestroy();
	afx_msg void OnShowPlacesWnd();
	afx_msg void OnShowStoresWnd();
	afx_msg void OnFocusMainWnd();
	afx_msg void OnToggleGrannyMode();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	afx_msg void OnToggleWorkflowWnd();
	afx_msg void OnTogglePlacesWnd();
	afx_msg void OnToggleStoreWnd();
	afx_msg void OnUpdatePaneCommands(CCmdUI* pCmdUI);
	afx_msg void OnToggleDeleteImported();
	afx_msg void OnToggleSimulate();
	afx_msg void OnUpdateMigrateCommands(CCmdUI* pCmdUI);
	afx_msg void OnStoreNew();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDefaultStoreChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLookChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	void ExecuteCreateStoreDlg(UINT nIDTemplate, char drv);
	void InitializeRibbon();
};
