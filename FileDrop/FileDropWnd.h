
// FileDropWnd.h: Headerdatei
//

#pragma once
#include "LFCommDlg.h"


// CFileDropWnd

class CFileDropWnd : public CGlassWindow
{
public:
	CFileDropWnd();
	~CFileDropWnd();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create();

protected:
	void UpdateStore();
	void SetWindowRect(int x, int y, BOOL TopMost);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnAlwaysOnTop();
	afx_msg void OnChooseDefaultStore();
	afx_msg void OnStoreProperties();
	afx_msg void OnAbout();
	afx_msg void OnNewStoreManager();
	afx_msg LRESULT OnStoresChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	LFDropTarget m_DropTarget;
	CSimpleTooltip Tooltip;
	HICON m_hIcon;
	CImageList m_Dropzone;
	CMFCToolBarImages m_Warning;
	LFStoreDescriptor m_Store;
	CString Label;
	BOOL AlwaysOnTop;
	BOOL MouseInWnd;
	BOOL Grabbed;
	BOOL StoreValid;
};
