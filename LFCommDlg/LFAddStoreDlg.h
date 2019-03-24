
// LFAddStoreDlg.h: Schnittstelle der Klasse LFAddStoreDlg
//

#pragma once
#include "CNotification.h"
#include "LFDialog.h"
#include "Box.h"
#include "Dropbox.h"
#include "GoogleDrive.h"
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
	void AddWindowsPathAsStore(LPCWSTR Path, LPCWSTR StoreName=L"");
	void AddGenericCloudProvider(const CString& ProviderName, LPCWSTR lpcPath);

	afx_msg void OnDestroy();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnDrawButtonForeground(UINT nCtrlID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequestTooltipData(UINT nCtrlID, NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBtnLiquidfolders();
	afx_msg void OnBtnWindows();
	afx_msg void OnBtnBox();
	afx_msg void OnBtnDropbox();
	afx_msg void OnBtnGoogleDrive();
	afx_msg void OnBtnICloud();
	afx_msg void OnBtnOneDrive();
	DECLARE_MESSAGE_MAP()

	static const UINT m_Sources[7];
	static CImageList m_SourceIcons;
	CNotification m_wndExplorerNotification;
	INT m_IconSize;

private:
	void CheckSources();

	Box m_Box;
	Dropbox m_Dropbox;
	GoogleDrive m_GoogleDrive;
	ICloud m_ICloud;
	OneDrive m_OneDrive;
};
