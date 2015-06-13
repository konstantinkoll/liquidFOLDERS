
// LFProgressDlg.cpp: Implementierung der Klasse LFProgressDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFProgressDlg
//

const GUID IID_ITaskbarList3 = { 0xEA1AFB91, 0x9E28, 0x4B86, {0x90, 0xE9, 0x9E, 0x9F, 0x8A, 0x5E, 0xEF, 0xAF}};

LFProgressDlg::LFProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd)
	: LFDialog(IDD_PROGRESS, pParentWnd)
{
	m_Abort = FALSE;
	m_pTaskbarList3 = NULL;

	p_ThreadProc = pThreadProc;
	p_Parameters = pParameters;

	ENSURE(m_XofY_Singular.LoadString(IDS_XOFY_SINGULAR));
	ENSURE(m_XofY_Plural.LoadString(IDS_XOFY_PLURAL));
}

void LFProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROGRESSBAR, m_wndProgress);
}


BEGIN_MESSAGE_MAP(LFProgressDlg, LFDialog)
	ON_WM_DESTROY()
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_MESSAGE(WM_UPDATEPROGRESS, OnUpdateProgress)
END_MESSAGE_MAP()

BOOL LFProgressDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	GetDlgItem(IDC_CAPTION)->SetWindowText(_T(""));
	GetDlgItem(IDC_PROGRESSCOUNT)->SetWindowText(_T(""));

	if (LFGetApp()->OSVersion>=OS_Seven)
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void**)&m_pTaskbarList3);

	if (p_ThreadProc)
	{
		ASSERT(p_Parameters);
		p_Parameters->hWnd = GetSafeHwnd();

		CreateThread(NULL, 0, p_ThreadProc, p_Parameters, 0, NULL);
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFProgressDlg::OnDestroy()
{
	if (m_pTaskbarList3)
	{
		m_pTaskbarList3->SetProgressState(GetSafeHwnd(), TBPF_NOPROGRESS);
		m_pTaskbarList3->Release();
	}
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

		// Taskbar
		if (m_pTaskbarList3)
			m_pTaskbarList3->SetProgressState(GetSafeHwnd(), TBPF_PAUSED);
	}
	else
	{
		EndDialog(IDCANCEL);
	}
}

LRESULT LFProgressDlg::OnUpdateProgress(WPARAM wParam, LPARAM /*lParam*/)
{
	LFProgress* pProgress = (LFProgress*)wParam;

	if (m_Abort)
	{
		pProgress->UserAbort = true;
		if (pProgress->ProgressState==LFProgressWorking)
			pProgress->ProgressState = LFProgressCancelled;
	}

	// Caption
	GetDlgItem(IDC_CAPTION)->SetWindowText(pProgress->Object);

	// Progress bar
	ASSERT(pProgress->ProgressState>=LFProgressWorking);
	ASSERT(pProgress->ProgressState<=LFProgressCancelled);
	m_wndProgress.SendMessage(0x410, pProgress->ProgressState);
	m_wndProgress.RedrawWindow();

	UINT nUpper;
	UINT nPos;
	UINT nCurrent;
	UINT nOf;

	ASSERT(pProgress->MinorCount>0);
	ASSERT(pProgress->MinorCurrent>=0);
	ASSERT(pProgress->MinorCurrent<=pProgress->MinorCount);

	if (pProgress->MajorCount>0)
	{
		ASSERT(pProgress->MajorCount>0);
		ASSERT(pProgress->MajorCurrent>=0);
		ASSERT(pProgress->MajorCurrent<max(1, pProgress->MajorCount));

		nUpper = pProgress->MajorCount*128;
		nPos = pProgress->MajorCurrent*128+(pProgress->MinorCurrent*128)/pProgress->MinorCount;
		nCurrent = pProgress->MajorCurrent+1;
		nOf = pProgress->MajorCount;
	}
	else
	{
		nUpper = pProgress->MinorCount;
		nPos = pProgress->MinorCurrent;
		nCurrent = pProgress->MinorCurrent+1;
		nOf = pProgress->NoMinorCounter ? 0 : pProgress->MinorCount;
	}

	m_wndProgress.SetRange32(0, nUpper);
	m_wndProgress.SetPos(nPos);

	// Taskbar
	if (m_pTaskbarList3)
	{
		m_pTaskbarList3->SetProgressState(GetSafeHwnd(), pProgress->ProgressState==LFProgressError ? TBPF_ERROR : pProgress->ProgressState==LFProgressCancelled ? TBPF_PAUSED : TBPF_NORMAL);
		m_pTaskbarList3->SetProgressValue(GetSafeHwnd(), nPos, nUpper);
	}

	// Counter
	CString tmpStr(_T(""));
	if ((nCurrent<=nOf) && (!m_Abort))
		tmpStr.Format(nOf==1 ? m_XofY_Singular : m_XofY_Plural, nCurrent, nOf);

	if (m_LastCounter!=tmpStr)
	{
		GetDlgItem(IDC_PROGRESSCOUNT)->SetWindowText(tmpStr);
		m_LastCounter = tmpStr;
	}

	return m_Abort;
}
