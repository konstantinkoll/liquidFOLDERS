
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "CFileView.h"
#include "CInspectorWnd.h"


// CMainView
//

class CMainView : public CWnd
{
friend class CInspectorWnd;

public:
	CMainView();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	BOOL Create(CWnd* pParentWnd, UINT nID, BOOL IsClipboard);
	void UpdateViewOptions();
	void UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data=NULL, BOOL UpdateSelection=TRUE);
	BOOL StoreIDValid();
	CHAR* GetStoreID();
	INT GetContext();
	INT GetViewID();
	void DismissNotification();
	void ShowNotification(UINT Type, CString Message, UINT Command=0);
	void ShowNotification(UINT Type, UINT ResID, UINT Command=0);
	INT GetSelectedItem();
	INT GetNextSelectedItem(INT n);
	void GetPersistentData(FVPersistentData& Data);
	void SelectNone();

protected:
	LFTransactionList* BuildTransactionList(BOOL All=FALSE, BOOL ResolvePhysicalLocations=FALSE, BOOL IncludePIDL=FALSE);
	void RemoveTransactedItems(LFTransactionList* pTransactionList);
	BOOL DeleteFiles(BOOL Trash, BOOL All=FALSE);
	BOOL RestoreFiles(UINT Flags, BOOL All=FALSE);
	BOOL UpdateItems(LFVariantData* Value1, LFVariantData* Value2, LFVariantData* Value3);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAdjustLayout();
	afx_msg LRESULT OnSetAlert(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateSelection();
	afx_msg void OnBeginDragDrop();
	afx_msg LRESULT OnRenameItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSendTo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam);

	afx_msg void OnToggleInspector();
	afx_msg void OnUpdatePaneCommands(CCmdUI* pCmdUI);

	afx_msg void OnSortOptions();
	afx_msg void OnToggleAutoDirs();
	afx_msg void OnUpdateHeaderCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnGetMenu(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSort(UINT nID);
	afx_msg void OnUpdateSortCommands(CCmdUI* pCmdUI);
	afx_msg void OnView(UINT nID);
	afx_msg void OnUpdateViewCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoresAdd();
	afx_msg void OnStoresMaintainAll();
	afx_msg void OnStoresShowStatistics();
	afx_msg void OnUpdateStoresCommands(CCmdUI* pCmdUI);

	afx_msg void OnNewRemoveNew();
	afx_msg void OnUpdateNewCommands(CCmdUI* pCmdUI);

	afx_msg void OnTrashRestoreAll();
	afx_msg void OnTrashEmpty();
	afx_msg void OnUpdateTrashCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateFiltersCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateItemCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreImportFolder();
	afx_msg void OnStoreShortcut();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);

	afx_msg void OnFileOpenWith();
	afx_msg void OnFileOpenBrowser();
	afx_msg void OnFileEdit();
	afx_msg void OnFileRemember();
	afx_msg void OnFileRemove();
	afx_msg void OnFileArchive();
	afx_msg void OnFileCopy();
	afx_msg void OnFileShortcut();
	afx_msg void OnFileDelete();
	afx_msg void OnFileRename();
	afx_msg void OnFileProperties();
	afx_msg void OnFileRestore();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CTaskbar m_wndTaskbar;
	CHeaderArea m_wndHeaderArea;
	CFileView* p_wndFileView;
	CInspectorWnd m_wndInspector;
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
	BOOL CreateFileView(UINT ViewID, FVPersistentData* Data);
	void SetHeaderButtons();
	void SetHeader();
	void AdjustLayout();
	void AddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData);

	CTaskButton* p_FilterButton;
	CTaskButton* p_InspectorButton;
	CHeaderButton* p_OrganizeButton;
	CHeaderButton* p_ViewButton;
	BOOL m_Resizing;
};
