
// LFAboutDlg.h: Schnittstelle der Klasse LFAboutDlg
//

#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"


// LFAboutDlg
//

class AFX_EXT_CLASS LFAboutDlg : public LFDialog
{
public:
	LFAboutDlg(CString AppName, CString Build, UINT IconResID, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnableAutoUpdate();
	afx_msg void OnUpdateNow();
	afx_msg void OnVersionInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

private:
	void CheckLicenseKey();
	void CheckInternetConnection();

	CGdiPlusBitmapResource m_Logo;
	CGdiPlusBitmapResource* m_pSanta;
	CFont m_CaptionFont;
	CFont m_VersionFont;
	INT m_CaptionTop;
	CString m_Version;
	CString m_Copyright;
	CString m_AppName;
	CString m_Build;
	CWnd m_wndVersionInfo;
};
