
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "CFileView.h"
#include "CInspectorPane.h"


// CMainView
//

#define WM_NAVIGATETO     WM_USER+202

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
	STOREID GetStoreID() const;
	BYTE GetContext() const;
	INT GetViewID() const;
	void DismissNotification();
	void ShowNotification(UINT Type, const CString& Message, UINT Command=0);
	void ShowNotification(UINT Type, UINT Result, UINT Command=0);
	void ShowNotification(UINT Result);
	void GetPersistentData(FVPersistentData& Data, BOOL ForReload=FALSE) const;
	void SelectNone();

protected:
	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

	LFTransactionList* BuildTransactionList(BOOL All=FALSE, BOOL ResolveLocations=FALSE, BOOL IncludePIDL=FALSE);
	void RemoveTransactedItems(LFTransactionList* pTransactionList);
	BOOL DeleteFiles(BOOL Trash, BOOL All=FALSE);
	void RecoverFiles(BOOL All=FALSE);
	BOOL UpdateItems(const LFVariantData* pValue1, const LFVariantData* pValue2, const LFVariantData* pValue3);
	INT GetSelectedItem() const;
	void SelectionChanged();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	afx_msg void OnDrawItem(INT nID, LPDRAWITEMSTRUCT lpdis);
	afx_msg void OnInitMenuPopup(CMenu* pMenuPopup, UINT nIndex, BOOL bSysMenu);

	afx_msg void OnAdjustLayout();
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginDragAndDrop(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnRenameItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnToggleInspector();
	afx_msg void OnUpdatePaneCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateHeaderCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnGetMenu(WPARAM wParam, LPARAM lParam);

	afx_msg void OnOrganizeButton();
	afx_msg void OnOrganizeOptions();
	afx_msg void OnSetOrganize(UINT nID);
	afx_msg void OnUpdateSetOrganizeCommands(CCmdUI* pCmdUI);

	afx_msg void OnViewButton();
	afx_msg void OnSetView(UINT nID);
	afx_msg void OnUpdateSetViewCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoresAdd();
	afx_msg void OnStoresSynchronize();
	afx_msg void OnStoresRunMaintenance();
	afx_msg void OnUpdateStoresCommands(CCmdUI* pCmdUI);

	afx_msg void OnFontsShowInstalled();
	afx_msg void OnUpdateFontsCommands(CCmdUI* pCmdUI);

	afx_msg void OnNewClearNew();
	afx_msg void OnUpdateNewCommands(CCmdUI* pCmdUI);

	afx_msg void OnTrashRecoverAll();
	afx_msg void OnTrashEmpty();
	afx_msg void OnUpdateTrashCommands(CCmdUI* pCmdUI);

	afx_msg void OnFiltersCreateNew();
	afx_msg void OnUpdateFiltersCommands(CCmdUI* pCmdUI);

	afx_msg void OnItemOpen();
	afx_msg void OnItemSendTo(UINT nID);
	afx_msg void OnItemShortcut();
	afx_msg void OnUpdateItemCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoreOpenNewWindow();
	afx_msg void OnStoreOpenFileDrop();
	afx_msg void OnStoreSynchronize();
	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);

	afx_msg void OnFileOpenWith();
	afx_msg void OnFileShowExplorer();
	afx_msg void OnFileEdit();
	afx_msg void OnFileRemember();
	afx_msg void OnFileRemoveFromClipboard();
	afx_msg void OnFileMakeTask();
	afx_msg void OnFileArchive();
	afx_msg void OnFileCompress();
	afx_msg void OnFileCopy();
	afx_msg void OnFileDelete();
	afx_msg void OnFileRename();
	afx_msg void OnFileProperties();
	afx_msg void OnFileTaskDone();
	afx_msg void OnFileRecover();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);

	afx_msg void OnFileMoveToContext(UINT CmdID);
	DECLARE_MESSAGE_MAP()

	static CIcons m_LargeIcons;
	static CIcons m_SmallIcons;
	CTaskbar m_wndTaskbar;
	CHeaderArea m_wndHeaderArea;
	CFileView* m_pWndFileView;
	CInspectorPane m_wndInspectorPane;
	CNotification m_wndExplorerNotification;
	LFDropTarget m_DropTarget;
	LFFilter* p_Filter;
	LFSearchResult* p_RawFiles;
	LFSearchResult* p_CookedFiles;
	STOREID m_StoreID;
	ITEMCONTEXT m_Context;
	INT m_ViewID;
	BOOL m_StoreIDValid;
	BOOL m_IsClipboard;
	BOOL m_ShowInspectorPane;
	BOOL m_Alerted;

private:
	BOOL CreateFileView(UINT ViewID, FVPersistentData* pPersistentData);
	void SetHeaderButtons();
	void SetHeader();
	void UpdateSearchResult();
	void AdjustLayout(UINT nFlags=SWP_NOACTIVATE | SWP_NOZORDER);

	CTaskButton* p_InspectorButton;
	CHeaderButton* p_OrganizeButton;
	CHeaderButton* p_ViewButton;
	BOOL m_Resizing;
};

inline BOOL CMainView::StoreIDValid() const
{
	return m_StoreIDValid;
}

inline STOREID CMainView::GetStoreID() const
{
	return m_StoreID;
}

inline BYTE CMainView::GetContext() const
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
	return m_pWndFileView && p_CookedFiles ? m_pWndFileView->GetSelectedItem() : -1;
}
