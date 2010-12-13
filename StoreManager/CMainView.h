
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

	// TODO
	CFileView* p_wndFileView;

protected:
	CTaskbar m_wndTaskbar;
	CExplorerHeader m_wndExplorerHeader;
	LFSearchResult* p_RawFiles;
	LFSearchResult* p_CookedFiles;
	INT m_Context;
	INT m_ViewID;
	BOOL m_ShowHeader;
	BOOL m_FilesSelected;

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
	afx_msg void OnUpdateStoresCommands(CCmdUI* pCmdUI);

	afx_msg void OnHomeImportFolder();
	afx_msg void OnHomeMaintain();
	afx_msg void OnHomeProperties();
	afx_msg void OnUpdateHomeCommands(CCmdUI* pCmdUI);

	afx_msg void OnUpdateHousekeepingCommands(CCmdUI* pCmdUI);

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

	DECLARE_MESSAGE_MAP()

private:
	BOOL CreateFileView(UINT ViewID, INT FocusItem);
	void AdjustLayout();
	void ExecuteContextMenu(CHAR Drive, LPCSTR verb);
};
