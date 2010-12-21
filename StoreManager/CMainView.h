
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "LFCommDlg.h"
#include "CFileView.h"


// CMainView
//

class CMainView : public CWnd
{
public:
	CMainView();
	~CMainView();

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	INT Create(CWnd* _pParentWnd, UINT nID);
	void UpdateViewOptions(INT Context);
	void UpdateSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, INT FocusItem);
	INT GetFocusItem();
	INT GetSelectedItem();
	INT GetNextSelectedItem(INT n);

protected:
	CTaskbar m_wndTaskbar;
	CExplorerHeader m_wndExplorerHeader;
	CFileView* p_wndFileView;
	LFSearchResult* p_RawFiles;
	LFSearchResult* p_CookedFiles;
	INT m_Context;
	INT m_ViewID;
	BOOL m_ShowHeader;
	BOOL m_FilesSelected;

	LFTransactionList* BuildTransactionList(BOOL All=FALSE);
	void RemoveTransactedItems(LFTransactionList* tl);
	BOOL UpdateTrashFlag(BOOL Trash, BOOL All=FALSE);
	BOOL DeleteFiles(BOOL All=FALSE);
	void ShowCaptionBar(LPCWSTR Icon, UINT res, UINT Command=0);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUpdateTaskbar(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnUpdateSelection();

	afx_msg void OnStoresCreateNew();
	afx_msg void OnStoresMaintainAll();
	afx_msg void OnStoresBackup();
	afx_msg void OnStoresHideEmptyDrives();
	afx_msg void OnUpdateStoresCommands(CCmdUI* pCmdUI);

	afx_msg void OnHomeHideEmptyDomains();
	afx_msg void OnHomeImportFolder();
	afx_msg void OnHomeMaintain();
	afx_msg void OnHomeProperties();
	afx_msg void OnUpdateHomeCommands(CCmdUI* pCmdUI);

	afx_msg void OnHousekeepingRegister();
	afx_msg void OnHousekeepingSend();
	afx_msg void OnUpdateHousekeepingCommands(CCmdUI* pCmdUI);

	afx_msg void OnTrashEmpty();
	afx_msg void OnTrashRestoreAll();
	afx_msg void OnUpdateTrashCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateItemCommands(CCmdUI* pCmdUI);

	afx_msg void OnDriveCreateNewStore();
	afx_msg void OnDriveFormat();
	afx_msg void OnDriveProperties();
	afx_msg void OnUpdateDriveCommands(CCmdUI* pCmdUI);

	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreMakeHybrid();
	afx_msg void OnStoreImportFolder();
	afx_msg void OnStoreMaintain();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);

	afx_msg void OnFileOpenWith();
	afx_msg void OnFileRemove();
	afx_msg void OnFileDelete();
	afx_msg void OnFileSend();
	afx_msg void OnFileRestore();
	afx_msg void OnUpdateFileCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	BOOL CreateFileView(UINT ViewID, INT FocusItem);
	void AdjustLayout();
	void ExecuteContextMenu(CHAR Drive, LPCSTR verb);
	void AddTransactionItem(LFTransactionList* tl, LFItemDescriptor* item, UINT UserData);
};
