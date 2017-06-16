
// LFAddStoreDlg.h: Schnittstelle der Klasse LFAddStoreDlg
//

#pragma once
#include "CExplorerNotification.h"
#include "LFDialog.h"
#include "Box.h"
#include "Dropbox.h"
#include "ICloud.h"
#include "OneDrive.h"


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
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnDrawButtonForeground(UINT nCtrlID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(UINT nCtrlID, NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg LRESULT OnUpdateStores(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBtnLiquidfolders();
	afx_msg void OnBtnWindows();
	afx_msg void OnBtnBox();
	afx_msg void OnBtnDropbox();
	afx_msg void OnBtnICloud();
	afx_msg void OnBtnOneDrive();
	DECLARE_MESSAGE_MAP()

	static const UINT m_Sources[6];
	static CImageList m_SourceIcons;
	CExplorerNotification m_wndExplorerNotification;
	INT m_IconSize;

private:
	void CheckSources();
	void AddWindowsPathAsStore(LPCWSTR Path, LPCWSTR StoreName=L"");

	Box m_Box;
	Dropbox m_Dropbox;
	ICloud m_ICloud;
	OneDrive m_OneDrive;
};
