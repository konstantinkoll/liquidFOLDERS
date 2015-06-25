
// CFileDropWnd.h: Schnittstelle der Klasse CFileDropWnd
//

#pragma once
#include "LFCommDlg.h"


// CFileDropWnd

#define WM_OPENFILEDROP     WM_USER+211

class CFileDropWnd : public CGlassWindow
{
public:
	CFileDropWnd();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CHAR* StoreID);

protected:
	void SetTopMost(BOOL AlwaysOnTop);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnOpenFileDrop(WPARAM wParam, LPARAM lParam);

	afx_msg void OnStoreOpen();
	afx_msg void OnStoreMakeDefault();
	afx_msg void OnStoreImportFolder();
	afx_msg void OnStoreShortcut();
	afx_msg void OnStoreDelete();
	afx_msg void OnStoreRename();
	afx_msg void OnStoreProperties();
	afx_msg void OnUpdateStoreCommands(CCmdUI* pCmdUI);

	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CHAR m_StoreID[LFKeySize];
	LFDropTarget m_DropTarget;
	LFTooltip m_TooltipCtrl;
	LFStoreDescriptor m_Store;
	CString m_Label;
	BOOL m_AlwaysOnTop;
	BOOL m_StoreValid;
	BOOL m_StoreMounted;
	BOOL m_Hover;
};