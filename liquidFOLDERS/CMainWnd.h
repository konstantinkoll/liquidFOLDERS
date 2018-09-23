
// CMainWnd.h: Schnittstelle der Klasse CMainWnd
//

#pragma once
#include "CJournalButton.h"
#include "CHistoryBar.h"
#include "CMainView.h"
#include "resource.h"


// CMainWnd
//

#define NAVMODE_NORMAL      0
#define NAVMODE_HISTORY     1
#define NAVMODE_CONTEXT     2
#define NAVMODE_RELOAD      3

#define WM_COOKFILES              WM_USER+203
#define WM_UPDATESORTSETTINGS     WM_COOKFILES
#define WM_UPDATEVIEWSETTINGS     WM_USER+204
#define WM_CONTEXTVIEWCOMMAND     WM_USER+205
#define WM_UPDATESIDEBAR          WM_USER+206

class CMainWnd : public CBackstageWnd
{
public:
	CMainWnd();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void GetLayoutRect(LPRECT lpRect);

	BOOL Create(LFFilter* pFilter=NULL, BOOL IsClipboard=FALSE);
	BOOL Create(const ABSOLUTESTOREID& StoreID);
	BOOL Create(LPCSTR IATACode);
	BOOL Create(BYTE ContextID);
	BOOL CreateClipboard();
	BOOL CreateRoot();
	BOOL AddClipItem(const LFItemDescriptor* pItemDescriptor, BOOL& First);

protected:
	virtual INT GetCaptionHeight(BOOL IncludeBottomMargin=TRUE) const;
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);

	void Reload();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSearchSetFocus();
	afx_msg void OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	afx_msg void OnDrawItem(INT nID, LPDRAWITEMSTRUCT lpdis);

	afx_msg void OnNavigateBack();
	afx_msg void OnNavigateForward();
	afx_msg void OnNavigateReload();
	afx_msg void OnUpdateNavigateCommands(CCmdUI* pCmdUI);

	afx_msg void OnSwitchContext(UINT nID);
	afx_msg void OnUpdateSwitchContextCommands(CCmdUI* pCmdUI);

	afx_msg LRESULT OnNavigateBack(WPARAM wParam, LPARAM lParam=NULL);
	afx_msg LRESULT OnNavigateTo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCookFiles(WPARAM wParam=NULL, LPARAM lParam=NULL);
	afx_msg void OnUpdateViewSettings();
	afx_msg LRESULT OnContextViewCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateSidebar();
	afx_msg void OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg LRESULT OnItemsDropped(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStatisticsChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	BOOL m_IsClipboard;
	CBackstageSidebar m_wndSidebar;
	CJournalButton m_wndJournalButton;
	CHistoryBar m_wndHistory;
	CBackstageEdit m_wndSearch;
	CMainView m_wndMainView;
	BreadcrumbItem* m_pBreadcrumbBack;
	BreadcrumbItem* m_pBreadcrumbForward;
	LFFilter* m_pActiveFilter;
	LFSearchResult* m_pRawFiles;
	LFSearchResult* m_pCookedFiles;
	LFStatistics m_Statistics;

private:
	static LFFilter* GetRootFilter();
	STOREID GetStatisticsID() const;
	void UpdateHistory(UINT NavMode);
	void NavigateTo(LFFilter* pFilter, UINT NavMode=NAVMODE_NORMAL, FVPersistentData* pPersistentData=NULL, INT AggregateFirst=-1, INT AggregateLast=-1);
	void LeafBreadcrumbs(BreadcrumbItem*& pAddItem, BreadcrumbItem*& pConsumeItem, UINT Pages=1);
	COLORREF PriorityColor() const;
	static BOOL CookSortDescending(const LFContextViewSettings* pContextViewSettings);
	static BOOL CookGroupSingle(const LFContextViewSettings* pContextViewSettings);

	static const UINT m_ContextOrder[LFLastQueryContext+1];
	STOREID m_StatisticsID;
	BOOL m_StatisticsResult;
};

inline LFFilter* CMainWnd::GetRootFilter()
{
	return LFAllocFilter(LFFilterModeStores);
}

inline BOOL CMainWnd::CreateClipboard()
{
	return Create(NULL, TRUE);
}

inline BOOL CMainWnd::CreateRoot()
{
	return Create(GetRootFilter());
}

inline void CMainWnd::Reload()
{
	PostMessage(WM_COMMAND, ID_NAV_RELOAD);
}

inline STOREID CMainWnd::GetStatisticsID() const
{
	ASSERT(m_pActiveFilter);

	return (m_pActiveFilter->Query.Mode<LFFilterModeQuery) && IsWindow(m_wndMainView) ? m_wndMainView.GetStoreID() : DEFAULTSTOREID();
}

inline COLORREF CMainWnd::PriorityColor() const
{
	return m_Statistics.TaskCount[10] ? 0x2020FF :
		m_Statistics.TaskCount[8] || m_Statistics.TaskCount[9] ? 0x0060FF :
		m_Statistics.TaskCount[6] || m_Statistics.TaskCount[7] ? 0x00A0E0 :
		m_Statistics.TaskCount[4] || m_Statistics.TaskCount[5] ? 0x05BB7D :
		m_Statistics.TaskCount[1] || m_Statistics.TaskCount[2] || m_Statistics.TaskCount[3] ? 0x069006 :
		(COLORREF)-1;
}

inline BOOL CMainWnd::CookSortDescending(const LFContextViewSettings* pContextViewSettings)
{
	return (pContextViewSettings->View==LFViewTimeline) ||
		(pContextViewSettings->SortDescending && (pContextViewSettings->View<=LFViewDetails));
}

inline BOOL CMainWnd::CookGroupSingle(const LFContextViewSettings* pContextViewSettings)
{
	const LFAttributeDescriptor* pAttribute = &LFGetApp()->m_Attributes[pContextViewSettings->SortBy];

	return ((pAttribute->AttrProperties.Type!=LFTypeTime) && (pContextViewSettings->SortBy!=LFAttrFileName)) ||
		(pContextViewSettings->View==LFViewCalendar) || (pContextViewSettings->View==LFViewGlobe) || (pContextViewSettings->View==LFViewTagcloud);
}
