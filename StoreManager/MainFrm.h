
// MainFrm.h: Schnittstelle der Klasse CMainFrame
//

#pragma once
#include "FilterWnd.h"
#include "InspectorWnd.h"
#include "HistoryWnd.h"
#include "liquidFOLDERS.h"
#include "CFileView.h"


// Ribbon
//

#define RibbonCategory_Home                 0
#define RibbonCategory_View                 1
#define RibbonCategory_Files                2
#define RibbonCategory_EMail_Mail           3
#define RibbonCategory_EMail_Contacts       4
#define RibbonCategory_View_Calendar        5
#define RibbonCategory_View_Globe           6
#define RibbonCategory_View_Tagcloud        7
#define RibbonCategory_View_Timeline        8
#define RibbonCategory_Stores               9
#define RibbonCategory_Trash                10
#define RibbonCategory_UnknownFileFormats   11

#define RibbonDefaultCategory               RibbonCategory_Files


class CAdvancedRibbonBar : public CMFCRibbonBar
{
public:
	CAdvancedRibbonBar()
	{
		ChangeOccured = FALSE;
	}

	void CAdvancedRibbonBar::ShowCategory(int nIndex, BOOL bShow=TRUE)
	{
		if (nIndex<this->GetCategoryCount())
			if (GetCategory(nIndex)->IsVisible()!=bShow)
			{
				CMFCRibbonBar::ShowCategory(nIndex, bShow);
				ChangeOccured = TRUE;
			}
	}

	void CAdvancedRibbonBar::Update()
	{
		if (ChangeOccured)
		{
			if (!GetActiveCategory()->IsVisible())
			{
				SetActiveCategory(GetCategory(RibbonDefaultCategory));
			}
			else
			{
				RecalcLayout();
			}
			
			ChangeOccured = FALSE;
		}
	}

private:
	BOOL ChangeOccured;
};


// CMainFrame
//

#define NAVMODE_NORMAL        0
#define NAVMODE_HISTORY       1
#define NAVMODE_RELOAD        2

class CMainFrame : public CFrameWndEx
{
public:
	CMainFrame(BOOL _IsClipboard = FALSE);
	virtual ~CMainFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	BOOL AddClipItem(LFItemDescriptor* i);
	void UpdateViewOptions();
	void UpdateSortOptions();
	BOOL RenameSingleItem(UINT n, CString Name);
	BOOL UpdateSelectedItems(LFVariantData* value1, wchar_t* ustr1=NULL, LFVariantData* value2=NULL, wchar_t* ustr2=NULL, LFVariantData* value3=NULL, wchar_t* ustr3=NULL);
	BOOL OpenChildView(BOOL Force=FALSE);

	BOOL IsClipboard;
	int ActiveContextID;

protected:
	void UpdateSearchResult(BOOL SetEmpty, int FocusItem);
	LFTransactionList* BuildTransactionList();
	UINT SelectViewMode(UINT ViewID=LFViewAutomatic);

	CAdvancedRibbonBar m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCRibbonComboBox* m_cbxActiveContext;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;
	CMFCRibbonBaseElement* m_sbItemCount;
	CMFCRibbonBaseElement* m_sbHint;
	CCaptionBar m_wndCaptionBar;
	CFilterWnd* m_wndFilter;
	CHistoryWnd* m_wndHistory;
	CInspectorWnd m_wndInspector;
	CFileView* m_wndView;
	BOOL FilesSelected;
	BreadcrumbItem* m_BreadcrumbBack;
	BreadcrumbItem* m_BreadcrumbForward;
	int CaptionBarUsed;
	int ActiveViewID;
	LFViewParameters* ActiveViewParameters;
	LFFilter* ActiveFilter;
	LFSearchResult* RawFiles;
	LFSearchResult* CookedFiles;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnAppCloseOthers();
	afx_msg void OnAppSortOptions();
	afx_msg void OnAppViewOptions();
/*	afx_msg void OnToggleAutoDirs();*/
	afx_msg void OnChooseContext();
	afx_msg void OnAlwaysSaveContext();
	afx_msg void OnRestoreContext();
	afx_msg void OnSaveContextNow();
	afx_msg void OnSaveContextAll();
	afx_msg void OnUpdateAppCommands(CCmdUI* pCmdUI);
	afx_msg void OnAppSort(UINT nID);
	afx_msg void OnUpdateAppSortCommands(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDropCommands(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavCommands(CCmdUI* pCmdUI);
	afx_msg void OnToggleCaptionBar();
	afx_msg void OnToggleFilterWnd();
	afx_msg void OnShowInspectorWnd();
	afx_msg void OnToggleInspectorWnd();
	afx_msg void OnShowHistoryWnd();
	afx_msg void OnToggleHistoryWnd();
	afx_msg void OnUpdatePaneCommands(CCmdUI* pCmdUI);
	afx_msg void OnClipRemove();
	afx_msg void OnClipRememberLast();
	afx_msg void OnClipRememberNew();
	afx_msg void OnUpdateClipCommands(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);
	afx_msg void OnStoreNew();
	afx_msg void OnStoreNewInternal();
	afx_msg void OnStoreNewDrive();
	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreMakeHybrid();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreBackupSelected();
	afx_msg void OnStoreBackupAll();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);
	afx_msg void OnChangeChildView(UINT nID);
	afx_msg void OnUpdateSelection();
	afx_msg void OnStartNavigation();
	afx_msg void OnNavigateFirst();
	afx_msg void OnNavigateBackOne();
	afx_msg LRESULT OnNavigateBack(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNavigateForward(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNavigateForwardOne();
	afx_msg void OnNavigateLast();
	afx_msg void OnNavigateReload();
	afx_msg void OnNavigateStores();
	afx_msg void OnNavigateHome();
	afx_msg void OnClearHistory();
	afx_msg LRESULT OnDrivesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLookChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL AttributeAllowedForSorting(int attr);
	void ExecuteCreateStoreDlg(UINT nIDTemplate, char drv);
	void Remember(CMainFrame* clip);
	void BackupStores(BOOL all=FALSE);
	int GetFocusItem();
	int GetSelectedItem();
	int GetNextSelectedItem(int n);
	int UpdateSingleItem(LFItemDescriptor* i1, LFItemDescriptor* i2, LFVariantData* value, wchar_t* string);
	void ShowCaptionBar(int Icon, LPCWSTR Message, int Command=0, LPCWSTR Button=_T(""));
	void ShowCaptionBar(int Icon, UINT res, int Command=0, LPCWSTR Button=_T(""));
	void InitializeRibbon();
	void NavigateTo(LFFilter* f, UINT NavMode=NAVMODE_NORMAL, int FocusItem=0);
	void CookFiles(int recipe, int FocusItem=0);
	void UpdateHistory();
};
