
// FileDropWnd.h: Headerdatei
//

#pragma once
#include "LFCommDlg.h"


// CFileDropWnd

class CFileDropWnd : public CGlasWindow
{
public:
	CFileDropWnd();
	~CFileDropWnd();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create();

protected:
	LFDropTarget m_DropTarget;
	LFTooltip m_TooltipCtrl;
	HICON m_hIcon;
	HICON m_hWarning;
	CImageList m_Dropzone;
	LFStoreDescriptor m_Store;
	CString m_Label;
	INT m_PosX;
	INT m_PosY;
	BOOL m_AlwaysOnTop;
	BOOL m_StoreValid;
	BOOL m_Hover;

	void UpdateStore();
	void SetWindowRect(INT x, INT y, BOOL TopMost);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnMove(INT x, INT y);
	afx_msg void OnAlwaysOnTop();
	afx_msg void OnChooseDefaultStore();
	afx_msg void OnImportFolder();
	afx_msg void OnStoreProperties();
	afx_msg void OnAbout();
	afx_msg void OnNewStoreManager();
	afx_msg void OnQuit();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
