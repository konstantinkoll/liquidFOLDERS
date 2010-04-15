
// FileDropDlg.h: Headerdatei
//

#pragma once
#include "LFCommDlg.h"
#include "LFDropTarget.h"


// CFileDropDlg-Dialogfeld

class CFileDropDlg : public CDialog
{
public:
	CFileDropDlg(CWnd* pParent = NULL);	// Standardkonstruktor

protected:
	HICON m_hIcon;
	CGdiPlusBitmapResource* dropzone;
	CGdiPlusBitmapResource* dropzoneL;
	CGdiPlusBitmapResource* dropzoneS;
	CGdiPlusBitmapResource* ready;
	CGdiPlusBitmapResource* warning;
	CGdiPlusBitmapResource* abouticon;
	CString strHint;
	CString strAbout;
	BOOL AlwaysOnTop;
	BOOL SmallWindow;
	BOOL liquidFOLDERSReady;
	UINT_PTR TimerID;
	HTHEME hTheme;
	BOOL Themed;
	CLFDropTarget m_DropTarget;

	void SetTopMost(BOOL TopMost);
	void SetWindowSize(BOOL Small);
	void UpdateStatus();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnAlwaysOnTop();
	afx_msg void OnSmallWindow();
	afx_msg void OnAbout();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnActivate(UINT,CWnd *,BOOL);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg void OnNcRButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//afx_msg void OnDropFiles(HDROP);
	afx_msg void OnTimer(UINT_PTR _TimerID);
	afx_msg void OnChooseDefaultStore();

	DECLARE_MESSAGE_MAP()
};
