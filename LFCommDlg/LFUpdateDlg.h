
// LFUpdateDlg.h: Schnittstelle der Klasse LFUpdateDlg
//

#pragma once
#include "LFCommDlg.h"


// LFUpdateDlg
//

class AFX_EXT_CLASS LFUpdateDlg : public LFDialog
{
public:
	LFUpdateDlg(CString Version, CString MSN, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	void UpdateFrame(BOOL bMove=FALSE);
	BOOL AddTrayIcon();
	BOOL RemoveTrayIcon();
	void ShowMenu();
	void EndDialog(INT_PTR nResult);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIgnoreUpdate();
	afx_msg void OnHide(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownload();
	afx_msg void OnCancel();
	afx_msg LRESULT OnTrayMenu(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRestore();
	DECLARE_MESSAGE_MAP()

private:
	void UpdateDownloadButton();
	void CheckInternetConnection();

	CGdiPlusBitmap* p_Logo;
	CFont m_CaptionFont;
	CFont m_VersionFont;
	INT m_CaptionTop;
	INT m_IconTop;
	CString m_AppName;
	CString m_Version;
	CString m_MSN;
	CWnd m_wndVersionInfo;
	CButton m_wndIgnoreUpdate;
	BOOL m_NotificationWindow;
	BOOL m_Connected;
};
