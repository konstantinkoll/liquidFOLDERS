
// LFProgressDlg.cpp: Implementierung der Klasse LFProgressDlg
//

#include "stdafx.h"
#include "LFProgressDlg.h"
#include "LFCore.h"
#include "Resource.h"
#include "..\\LFCore\\resource.h"

using namespace Gdiplus;


// LFProgressDlg
//

LFProgressDlg::LFProgressDlg(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParent)
	: LFDialog(IDD_PROGRESS, LFDS_White, pParent)
{
	m_Abort = FALSE;

	p_ThreadProc = pThreadProc;
	p_Parameters = pParameters;
}

void LFProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PROGRESSBAR, m_wndProgress);
}


BEGIN_MESSAGE_MAP(LFProgressDlg, LFDialog)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_MESSAGE(WM_UPDATEPROGRESS, OnUpdateProgress)
END_MESSAGE_MAP()

BOOL LFProgressDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	GetDlgItem(IDC_CAPTION)->SetWindowText(_T(""));

	if (p_ThreadProc)
	{
		ASSERT(p_Parameters);
		p_Parameters->hWnd = GetSafeHwnd();

		CreateThread(NULL, 0, p_ThreadProc, p_Parameters, 0, NULL);
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFProgressDlg::OnCancel()
{
	m_Abort = TRUE;

	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
}

LRESULT LFProgressDlg::OnUpdateProgress(WPARAM wParam, LPARAM /*lParam*/)
{
	LFProgress* pProgress = (LFProgress*)wParam;

	// Caption
	GetDlgItem(IDC_CAPTION)->SetWindowText(pProgress->Object);

	// Progress bar
	ASSERT(pProgress->ProgressState>=LFProgressWorking);
	ASSERT(pProgress->ProgressState<=LFProgressPaused);
	m_wndProgress.SendMessage(0x410, pProgress->ProgressState);

	UINT nUpper;
	UINT nPos;

	ASSERT(pProgress->MinorCount>0);
	ASSERT(pProgress->MinorCurrent>=0);
	ASSERT(pProgress->MinorCurrent<max(1, pProgress->MinorCount));

	if (pProgress->MajorCount>0)
	{
		ASSERT(pProgress->MajorCount>0);
		ASSERT(pProgress->MajorCurrent>=0);
		ASSERT(pProgress->MajorCurrent<max(1, pProgress->MajorCount));

		nUpper = pProgress->MajorCount*128;
		nPos = pProgress->MajorCurrent*128+(pProgress->MinorCurrent*128)/pProgress->MinorCount;
	}
	else
	{
		nUpper = pProgress->MinorCount;
		nPos = pProgress->MinorCurrent;
	}

	m_wndProgress.SetRange32(0, nUpper);
	m_wndProgress.SetPos(nPos);

	return m_Abort;
}
