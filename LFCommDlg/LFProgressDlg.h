
// LFProgressDlg.h: Schnittstelle der Klasse LFProgressDlg
//

#pragma once
#include "LFDialog.h"


// LFProgressDlg
//

struct LFWorkerParameters
{
	HWND hWnd;
};

class LFProgressDlg : public LFDialog
{
public:
	LFProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	void UpdateProgress();

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CString m_XofY_Singular;
	CString m_XofY_Plural;

private:
	LPTHREAD_START_ROUTINE p_ThreadProc;
	LFWorkerParameters* p_Parameters;
	CMutex m_Mutex;
	LFProgress m_Progress;
	CProgressCtrl m_wndProgress;
	BOOL m_Abort;
};
