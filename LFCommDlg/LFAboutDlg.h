
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
	virtual void CheckLicenseKey(LFLicense* License=NULL);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnEnableAutoUpdate();
	afx_msg void OnUpdateNow();
	DECLARE_MESSAGE_MAP()

private:
	CString m_Version;
	CString m_Copyright;
	CString m_AppName;
	CString m_Build;
	CGdiPlusBitmapResource m_Icon;
};
