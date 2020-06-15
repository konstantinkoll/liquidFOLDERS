
// LFMakeTaskDlg.cpp: Implementierung der Klasse LFMakeTaskDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFMakeTaskDlg.h"


// LFMakeTaskDlg
//

LFMakeTaskDlg::LFMakeTaskDlg(LFVariantData* pVDataPriority, LFVariantData* pVDataDueTime, CWnd* pParentWnd)
	: LFPickTimeDlg(pVDataDueTime, LFContextTasks, pParentWnd, IDD_MAKETASK, FALSE, FALSE)
{
	ASSERT(pVDataPriority);

	p_VDataPriority = pVDataPriority;
}

void LFMakeTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	LFPickTimeDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PRIORITY, m_wndPriority);

	if (pDX->m_bSaveAndValidate)
		*p_VDataPriority = m_wndPriority.m_VData;
}

BOOL LFMakeTaskDlg::InitDialog()
{
	LFPickTimeDlg::InitDialog();

	// Priority control
	CRect rectCalendar;
	m_wndCalendar.GetWindowRect(rectCalendar);

	CRect rect;
	m_wndPriority.GetWindowRect(rect);
	m_wndPriority.SetWindowPos(NULL, 0, 0, rectCalendar.Width(), rect.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);

	m_wndPriority.SetInitialData(*p_VDataPriority);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFMakeTaskDlg, LFPickTimeDlg)
	ON_BN_CLICKED(IDC_USEDATE, OnUseDate)
END_MESSAGE_MAP()

void LFMakeTaskDlg::OnUseDate()
{
	m_UseDate = ((CButton*)GetDlgItem(IDC_USEDATE))->GetCheck();

	EnableControls();
}
