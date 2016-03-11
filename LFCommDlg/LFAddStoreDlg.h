
// LFAddStoreDlg.h: Schnittstelle der Klasse LFAddStoreDlg
//

#pragma once
#include "CExplorerNotification.h"
#include "LFDialog.h"


// LFAddStoreDlg
//

class LFAddStoreDlg : public LFDialog
{
public:
	LFAddStoreDlg(CWnd* pParentWnd=NULL);

	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);

protected:
	virtual BOOL InitDialog();

	void ShowResult(UINT Result, const CString StoreName);

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDrawButtonForeground(UINT nCtrlID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnLiquidfolders();
	afx_msg void OnBtnWindows();
	DECLARE_MESSAGE_MAP()

	CExplorerNotification m_wndExplorerNotification;

private:
	void CheckInternetConnection();
};
