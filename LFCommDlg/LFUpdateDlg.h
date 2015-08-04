
// LFUpdateDlg.h: Schnittstelle der Klasse LFUpdateDlg
//

#pragma once
#include "CGdiPlusBitmap.h"
#include "LFDialog.h"


// LFUpdateDlg
//

#define UPDATE_SECUTIRYPATCH         1
#define UPDATE_IMPORTANTBUGFIX       2
#define UPDATE_NETWORKAPI            4
#define UPDATE_NEWFEATURE            8
#define UPDATE_NEWVISUALIZATION      16
#define UPDATE_UI                    32
#define UPDATE_SMALLBUGFIX           64
#define UPDATE_IATA                  128
#define UPDATE_PERFORMANCE           256

class LFUpdateDlg : public LFDialog
{
public:
	LFUpdateDlg(CString Version, CString MSN, DWORD Features, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	void UpdateFrame(BOOL bMove=FALSE);
	BOOL AddTrayIcon();
	BOOL RemoveTrayIcon();
	void ShowMenu();
	void EndDialog(INT nResult);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void PostNcDestroy();
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
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()

private:
	void UpdateDownloadButton();
	void CheckInternetConnection();

	CGdiPlusBitmap* m_pLogo;
	CImageListTransparent m_UpdateIcons;
	CFont m_CaptionFont;
	CFont m_VersionFont;
	INT m_CaptionTop;
	INT m_IconTop;
	INT m_FeaturesTop;
	INT m_FeaturesLeft;
	INT m_FeatureItemHeight;
	CString m_AppName;
	CString m_Version;
	CString m_MSN;
	DWORD m_Features;
	CWnd m_wndVersionInfo;
	CButton m_wndIgnoreUpdate;
	BOOL m_NotificationWindow;
	BOOL m_Connected;
};
