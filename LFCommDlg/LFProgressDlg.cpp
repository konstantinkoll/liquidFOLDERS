f
// LFProgressDlg.cpp: Implementierung der Klasse LFProgressDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFProgressDlg
//

LFProgressDlg::LFProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd)
	: LFDialog(IDD_PROGRESS, pParentWnd, TRUE)
{
	ZeroMemory(&m_Progress, sizeof(m_Progress));
	m_Abort = m_Update = FALSE;

	p_ThreadProc = pThreadProc;
	p_Parameters = pParameters;

	ENSURE(m_XofY_Singular.LoadString(IDS_XOFY_SINGULAR));
	ENSURE(m_XofY_Plural.LoadString(IDS_XOFY_PLURAL));
}

void LFProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROGRESSBAR, m_wndProgress);
}

void LFProgressDlg::UpdateProgress()
{
	// Caption
	GetDlgItem(IDC_CAPTION)->SetWindowText(m_Progress.Object);

	// Progress bar
	ASSERT(m_Progress.ProgressState>=LFProgressWorking);
	ASSERT(m_Progress.ProgressState<=LFProgressCancelled);

	m_wndProgress.SendMessage(0x410, m_Progress.ProgressState);

	UINT nUpper;
	UINT nPos;
	UINT nCurrent;
	UINT nOf;

	ASSERT(m_Progress.MinorCount>0);
	ASSERT(m_Progress.MinorCurrent>=0);
	ASSERT(m_Progress.MinorCurrent<=m_Progress.MinorCount);

	if (m_Progress.MajorCount>0)
	{
		ASSERT(m_Progress.MajorCount>0);
		ASSERT(m_Progress.MajorCurrent>=0);
		ASSERT(m_Progress.MajorCurrent<max(1, m_Progress.MajorCount));

		nUpper = m_Progress.MajorCount*128;
		nPos = m_Progress.MajorCurrent*128+(m_Progress.MinorCurrent*128)/m_Progress.MinorCount;
		nCurrent = m_Progress.MajorCurrent+1;
		nOf = m_Progress.MajorCount;
	}
	else
	{
		nUpper = m_Progress.MinorCount;
		nPos = m_Progress.MinorCurrent;
		nCurrent = m_Progress.MinorCurrent+1;
		nOf = m_Progress.NoMinorCounter ? 0 : m_Progress.MinorCount;
	}

	m_wndProgress.SetRange32(0, nUpper);
	m_wndProgress.SetPos(nPos);

	// Taskbar
	LFSetTaskbarProgress(this, nPos, nUpper, m_Progress.ProgressState==LFProgressError ? TBPF_ERROR : m_Progress.ProgressState==LFProgressCancelled ? TBPF_PAUSED : TBPF_NORMAL);

	// Counter
	CString tmpStr(_T(""));
	if ((nCurrent<=nOf) && !m_Abort)
		tmpStr.Format(nOf==1 ? m_XofY_Singular : m_XofY_Plural, nCurrent, nOf);

	GetDlgItem(IDC_PROGRESSCOUNT)->SetWindowText(tmpStr);

	m_Update = FALSE;
}

BOOL LFProgressDlg::InitDialog()
{
	if (p_ThreadProc)
	{
		ASSERT(p_Parameters);
		p_Parameters->hWnd = GetSafeHwnd();

		CreateThread(NULL, 0, p_ThreadProc, p_Parameters, 0, NULL);
	}

	SetTimer(1, 50, NULL);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFProgressDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->UpdateProgress, OnUpdateProgress)
END_MESSAGE_MAP()

void LFProgressDlg::OnDestroy()
{
	KillTimer(1);

	LFDialog::OnDestroy();
}

void LFProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	if ((nIDEvent==1) && m_Update)
		UpdateProgress();

	LFDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void LFProgressDlg::OnOK()
{
	LFHideTaskbarProgress(this);

	LFDialog::OnOK();
}

void LFProgressDlg::OnCancel()
{
	if (p_ThreadProc)
	{
		m_Abort = TRUE;
		GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

		// Progress bar
		m_wndProgress.SendMessage(0x410, LFProgressCancelled);
		m_wndProgress.RedrawWindow();
	}
	else
	{
		LFDialog::OnCancel();
	}
}

LRESULT LFProgressDlg::OnUpdateProgress(WPARAM wParam, LPARAM /*lParam*/)
{
	LFProgress* pProgress = (LFProgress*)wParam;

	if (!pProgress)
		return FALSE;

	// Handle abort
	if (m_Abort)
	{
		pProgress->UserAbort = TRUE;

		if (pProgress->ProgressState==LFProgressWorking)
			pProgress->ProgressState = LFProgressCancelled;
	}

	// Copy progress data
	CSingleLock ProgressLock(&m_Mutex);
	ProgressLock.Lock();

	m_Progress = *pProgress;
	m_Update = TRUE;

	ProgressLock.Unlock();

	return m_Abort;
}
