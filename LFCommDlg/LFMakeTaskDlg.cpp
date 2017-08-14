
// LFMakeTaskDlg.cpp: Implementierung der Klasse LFMakeTaskDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFMakeTaskDlg.h"


// LFMakeTaskDlg
//

LFMakeTaskDlg::LFMakeTaskDlg(LFVariantData* pDataPriority, LFVariantData* pDataDueTime, CWnd* pParentWnd)
	: LFEditTimeDlg(pDataDueTime, pParentWnd, IDD_MAKETASK, FALSE, FALSE)
{
	ASSERT(pDataPriority);

	p_Data = pDataPriority;
}

void LFMakeTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	LFEditTimeDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PRIORITY, m_wndPriority);

	if (pDX->m_bSaveAndValidate)
		*p_Data = m_wndPriority.m_Data;
}

BOOL LFMakeTaskDlg::InitDialog()
{
	LFEditTimeDlg::InitDialog();

	// Size
	CRect rectCalendar;
	m_wndCalendar.GetWindowRect(rectCalendar);

	CRect rect;
	m_wndPriority.GetWindowRect(rect);
	m_wndPriority.SetWindowPos(NULL, 0, 0, rectCalendar.Width(), rect.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);

	// Data
	m_wndPriority.SetData(*p_Data);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFMakeTaskDlg, LFEditTimeDlg)
	ON_BN_CLICKED(IDC_USEDATE, OnUseDate)
END_MESSAGE_MAP()

void LFMakeTaskDlg::OnUseDate()
{
	m_UseDate = ((CButton*)GetDlgItem(IDC_USEDATE))->GetCheck();
	EnableControls();
}
