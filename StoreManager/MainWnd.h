
// MainWnd.h: Schnittstelle der Klasse CMainWnd
//

#pragma once
#include "liquidFOLDERS.h"
#include "CJournalButton.h"
#include "CHistoryBar.h"
#include "CMainView.h"
#include "CFileView.h"


// CMainWnd
//

#define NAVMODE_NORMAL           0
#define NAVMODE_HISTORY          1
#define NAVMODE_RELOAD           2

#define WM_UPDATEVIEWOPTIONS     WM_USER+200
#define WM_UPDATESORTOPTIONS     WM_USER+201
#define WM_RELOAD                WM_USER+202
#define WM_COOKFILES             WM_USER+203
#define WM_UPDATEFOOTER          WM_USER+204

class CMainWnd : public CGlasWindow
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL Create(BOOL IsClipboard, CHAR* RootStore=NULL);
	BOOL AddClipItem(LFItemDescriptor* i);
	BOOL UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);

	BOOL m_IsClipboard;
	INT ActiveContextID;
	INT ActiveViewID;

protected:
	HICON m_hIcon;
	CJournalButton m_wndJournalButton;
	CHistoryBar m_wndHistory;
	CMainView m_wndMainView;
	BreadcrumbItem* m_BreadcrumbBack;
	BreadcrumbItem* m_BreadcrumbForward;
	LFViewParameters* ActiveViewParameters;
	LFFilter* ActiveFilter;
	LFSearchResult* RawFiles;
	LFSearchResult* CookedFiles;

	void UpdateSearchResult(BOOL SetEmpty, FVPersistentData* Data);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnNavigateBack();
	afx_msg void OnNavigateForward();
	afx_msg void OnNavigateReload();
	afx_msg void OnUpdateNavCommands(CCmdUI* pCmdUI);

	afx_msg void OnItemOpen();

	afx_msg void OnUpdateViewOptions();
	afx_msg void OnUpdateSortOptions();
	afx_msg LRESULT OnCookFiles(WPARAM wParam=0, LPARAM lParam=NULL);
	afx_msg void OnUpdateFooter();

	afx_msg LRESULT OnDrivesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	void Remember(CMainWnd* clip);
	void InitializeRibbon();
	void NavigateTo(LFFilter* f, UINT NavMode=NAVMODE_NORMAL, FVPersistentData* Data=NULL, INT FirstAggregate=-1, INT LastAggregate=-1);
	void UpdateHistory();
};
