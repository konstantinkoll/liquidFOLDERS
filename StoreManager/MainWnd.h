
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
#define WM_NAVIGATEBACK          WM_USER+205
#define WM_NAVIGATETO            WM_USER+206
#define WM_SENDTO                WM_USER+207
#define WM_BEGINDRAGDROP         WM_USER+208

class CMainWnd : public CGlassWindow
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();

	BOOL CreateClipboard();
	BOOL CreateRoot();
	BOOL CreateStore(CHAR* RootStore);
	BOOL CreateFilter(LFFilter* f);
	BOOL CreateFilter(WCHAR* FileName);
	INT GetContext();
	INT GetViewID();
	BOOL AddClipItem(LFItemDescriptor* i);

	BOOL m_IsClipboard;

protected:
	HICON m_hIcon;
	CJournalButton m_wndJournalButton;
	CHistoryBar m_wndHistory;
	CGlassEdit m_wndSearch;
	CSidebar m_wndSidebar;
	CMainView m_wndMainView;
	BreadcrumbItem* m_BreadcrumbBack;
	BreadcrumbItem* m_BreadcrumbForward;
	LFFilter* m_pActiveFilter;
	LFSearchResult* m_pRawFiles;
	LFSearchResult* m_pCookedFiles;
	BOOL m_ShowFilterPane;

	BOOL Create(BOOL IsClipboard);
	void HideFilterPane();
	void WriteMetadataTXT(CStdioFile& f);
	void WriteMetadataXML(CStdioFile& f);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnNavigateBack();
	afx_msg LRESULT OnNavigateBack(WPARAM wParam, LPARAM lParam=NULL);
	afx_msg void OnNavigateForward();
	afx_msg void OnNavigateReload();
	afx_msg void OnNavigateSwitchContext(UINT nID);
	afx_msg void OnUpdateNavCommands(CCmdUI* pCmdUI);

	afx_msg void OnToggleFilterPane();

	afx_msg void OnItemOpen();
	afx_msg void OnItemOpenNewWindow();
	afx_msg LRESULT OnNavigateTo(WPARAM wParam, LPARAM lParam);

	afx_msg void OnExportMetadata();

	afx_msg void OnUpdateViewOptions();
	afx_msg void OnUpdateSortOptions();
	afx_msg LRESULT OnCookFiles(WPARAM wParam=0, LPARAM lParam=NULL);
	afx_msg void OnUpdateFooter();

	afx_msg LRESULT OnVolumesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnItemsDropped(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	void NavigateTo(LFFilter* f, UINT NavMode=NAVMODE_NORMAL, FVPersistentData* Data=NULL, INT FirstAggregate=-1, INT LastAggregate=-1);
	void UpdateHistory();
};
