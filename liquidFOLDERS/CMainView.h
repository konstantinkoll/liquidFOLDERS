
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "CFileView.h"
#include "CInspectorPane.h"


// CMainView
//

class CMainView : public CFrontstageWnd
{
friend class CInspectorPane;

public:
	CMainView();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL IsClipboard);
	void UpdateViewSettings();
	void UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData=NULL, BOOL UpdateSelection=TRUE);
	BOOL StoreIDValid() const;
	LPCSTR GetStoreID() const;
	INT GetContext() const;
	INT GetViewID() const;
	void DismissNotification();
	void ShowNotification(UINT Type, const CString& Message, UINT Command=0);
	void ShowNotification(UINT Type, UINT Result, UINT Command=0);
	void ShowNotification(UINT Result);
	INT GetSelectedItem() const;
	void GetPersistentData(FVPersistentData& Data, BOOL ForReload=FALSE) const;
	void SelectNone();

protected:
	LFTransactionList* BuildTransactionList(BOOL All=FALSE, BOOL ResolveLocations=FALSE, BOOL IncludePIDL=FALSE);
	void RemoveTransactedItems(LFTransactionList* pTransactionList);
	BOOL DeleteFiles(BOOL Trash, BOOL All=FALSE);
	void RecoverFiles(BOOL All=FALSE);
	BOOL UpdateItems(LFVariantData* pValue1, LFVariantData* pValue2, LFVariantData* pValue3);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	afx_msg void OnDrawItem(INT nID, LPDRAWITEMSTRUCT lpdis);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAdjustLayout();
	afx_msg void OnUpdateSelection();
	afx_msg void OnBeginDragDrop();
	afx_msg LRESULT OnRenameItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSendTo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnToggleInspector();
	afx_msg void OnUpdatePaneCommands(CCmdUI* pCmdUI);

	afx_msg void OnSortOptions();
	afx_msg void OnUpdateHeaderCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnGetMenu(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSort(UINT nID);
	afx_msg void OnUpdateSortCommands(CCmdUI* pCmdUI);
	afx_msg void OnView(UINT nID);
	afx_msg void OnUpdateViewCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoresAdd();
	afx_msg void OnStoresSynchronize();
	afx_msg void OnStoresRunMaintenance();
	afx_msg void OnUpdateStoresCommands(CCmdUI* pCmdUI);

	afx_msg void OnNewClearNew();
	afx_msg void OnUpdateNewCommands(CCmdUI* pCmdUI);

	afx_msg void OnTrashRecoverAll();
	afx_msg void OnTrashEmpty();
	afx_msg void OnUpdateTrashCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateFiltersCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateItemCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoreSynchronize();
	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreShortcut();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);

	afx_msg void OnFileOpenWith();
	afx_msg void OnFileOpenBrowser();
	afx_msg void OnFileEdit();
	afx_msg void OnFileRemember();
	afx_msg void OnFileRemoveFromClipboard();
	afx_msg void OnFileMakeTask();
	afx_msg void OnFileArchive();
	afx_msg void OnFileCopy();
	afx_msg void OnFileShortcut();
	afx_msg void OnFileDelete();
	afx_msg void OnFileRename();
	afx_msg void OnFileProperties();
	afx_msg void OnFileTaskDone();
	afx_msg void OnFileRecover();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CTaskbar m_wndTaskbar;
	CHeaderArea m_wndHeaderArea;
	CFileView* m_pWndFileView;
	CInspectorPane m_wndInspectorPane;
	CExplorerNotification m_wndExplorerNotification;
	LFDropTarget m_DropTarget;
	LFFilter* p_Filter;
	LFSearchResult* p_RawFiles;
	LFSearchResult* p_CookedFiles;
	CHAR m_StoreID[LFKeySize];
	INT m_Context;
	INT m_ViewID;
	BOOL m_StoreIDValid;
	BOOL m_IsClipboard;
	BOOL m_FilesSelected;
	BOOL m_ShowInspectorPane;
	BOOL m_Alerted;

private:
	BOOL CreateFileView(UINT ViewID, FVPersistentData* pPersistentData);
	void SetHeaderButtons();
	void SetHeader();
	void UpdateSearchResult();
	void AdjustLayout(UINT nFlags=SWP_NOACTIVATE | SWP_NOZORDER);
	static void CreateShortcut(LFTransactionListItem* pItem);

	CTaskButton* p_InspectorButton;
	CHeaderButton* p_OrganizeButton;
	CHeaderButton* p_ViewButton;
	BOOL m_Resizing;
};

inline BOOL CMainView::StoreIDValid() const
{
	return m_StoreIDValid;
}

inline LPCSTR CMainView::GetStoreID() const
{
	return m_StoreID;
}

inline INT CMainView::GetContext() const
{
	return m_Context;
}

inline INT CMainView::GetViewID() const
{
	return m_ViewID;
}

inline void CMainView::DismissNotification()
{
	m_wndExplorerNotification.DismissNotification();
}

inline void CMainView::ShowNotification(UINT Type, const CString& Message, UINT Command)
{
	m_wndExplorerNotification.SetNotification(Type, Message, Command);
}

inline INT CMainView::GetSelectedItem() const
{
	return m_pWndFileView ? m_pWndFileView->GetSelectedItem() : -1;
}
