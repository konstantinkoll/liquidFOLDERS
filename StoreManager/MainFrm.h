
// MainFrm.h: Schnittstelle der Klasse CMainFrame
//

#pragma once
#include "FilterWnd.h"
#include "InspectorWnd.h"
#include "HistoryWnd.h"
#include "liquidFOLDERS.h"
#include "CMainView.h"
#include "CFileView.h"


// CMainFrame
//

#define NAVMODE_NORMAL        0
#define NAVMODE_HISTORY       1
#define NAVMODE_RELOAD        2

#define WM_UPDATEVIEWOPTIONS     WM_USER+200
#define WM_UPDATESORTOPTIONS     WM_USER+201
#define WM_RELOAD                WM_USER+202
#define WM_COOKFILES             WM_USER+203
#define WM_UPDATESEARCHRESULT    WM_USER+204

class CMainFrame : public CFrameWndEx
{
public:
	CMainFrame(char* RootStore=NULL, BOOL _IsClipboard=FALSE);
	virtual ~CMainFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	BOOL AddClipItem(LFItemDescriptor* i);
	BOOL UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);
	void OnUpdateSelection();

	BOOL IsClipboard;
	INT ActiveContextID;
	INT ActiveViewID;

protected:
	void UpdateSearchResult(BOOL SetEmpty, INT FocusItem);

	CMFCRibbonBar m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar m_wndStatusBar;
	CFilterWnd* m_wndFilter;
	CHistoryWnd* m_wndHistory;
	CInspectorWnd m_wndInspector;
	CMainView m_wndMainView;
	BreadcrumbItem* m_BreadcrumbBack;
	BreadcrumbItem* m_BreadcrumbForward;
	LFViewParameters* ActiveViewParameters;
	LFFilter* ActiveFilter;
	LFSearchResult* RawFiles;
	LFSearchResult* CookedFiles;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnCloseOthers();
	afx_msg void OnSortOptions();
	afx_msg void OnViewOptions();
	afx_msg void OnToggleAutoDirs();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	afx_msg void OnSort(UINT nID);
	afx_msg void OnUpdateSortCommands(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDropCommands(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavCommands(CCmdUI* pCmdUI);
	afx_msg void OnToggleFilterWnd();
	afx_msg void OnToggleInspectorWnd();
	afx_msg void OnShowHistoryWnd();
	afx_msg void OnToggleHistoryWnd();
	afx_msg void OnUpdatePaneCommands(CCmdUI* pCmdUI);
	afx_msg void OnChangeChildView(UINT nID);
	afx_msg void OnNavigateBack();
	afx_msg void OnNavigateForward();
	afx_msg void OnNavigateReload();
	afx_msg void OnNavigateStores();
	afx_msg void OnNavigateHome();

	afx_msg void OnItemOpen();

	afx_msg void OnUpdateViewOptions();
	afx_msg void OnUpdateSortOptions();
	afx_msg LRESULT OnCookFiles(WPARAM wParam=0, LPARAM lParam=NULL);
	afx_msg void OnUpdateSearchResult();

	afx_msg LRESULT OnDrivesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL AttributeAllowedForSorting(INT attr);
	void Remember(CMainFrame* clip);
	void InitializeRibbon();
	void NavigateTo(LFFilter* f, UINT NavMode=NAVMODE_NORMAL, INT FocusItem=0, INT FirstAggregate=-1, INT LastAggregate=-1);
	void UpdateHistory();
};
