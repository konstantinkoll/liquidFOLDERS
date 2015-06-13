
// LFProgressDlg.h: Schnittstelle der Klasse LFProgressDlg
//

#pragma once
#include "LFDialog.h"
#include "ITaskbarList3.h"


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

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnCancel();
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	CString m_XofY_Singular;
	CString m_XofY_Plural;
	ITaskbarList3* m_pTaskbarList3;

private:
	BOOL m_Abort;
	LPTHREAD_START_ROUTINE p_ThreadProc;
	LFWorkerParameters* p_Parameters;
	CProgressCtrl m_wndProgress;
	CString m_LastCounter;
};
