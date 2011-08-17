
// LFProgressDlg.h: Schnittstelle der Klasse LFProgressDlg
//

#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"


// LFProgressDlg
//

struct LFWorkerParameters
{
	HWND hWnd;
};

class AFX_EXT_CLASS LFProgressDlg : public LFDialog
{
public:
	LFProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CString m_XofY_Singular;
	CString m_XofY_Plural;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_Abort;
	LPTHREAD_START_ROUTINE p_ThreadProc;
	LFWorkerParameters* p_Parameters;
	CProgressCtrl m_wndProgress;
	CString m_LastCounter;
};
