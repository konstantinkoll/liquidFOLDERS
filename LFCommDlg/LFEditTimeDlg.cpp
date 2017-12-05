
// LFEditTimeDlg.cpp: Implementierung der Klasse LFEditTimeDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditTimeDlg.h"


// LFEditTimeDlg
//

LFEditTimeDlg::LFEditTimeDlg(LFVariantData* pVData, CWnd* pParentWnd, UINT nIDTemplate, BOOL UseTime, BOOL UseDate)
	: LFDialog(nIDTemplate, pParentWnd)
{
	ASSERT(pVData);

	p_VData = pVData;
	m_UseDate = UseDate;
	m_UseTime = UseTime;
}

void LFEditTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CALENDAR, m_wndCalendar);
	DDX_Control(pDX, IDC_TIME, m_wndTime);

	if (pDX->m_bSaveAndValidate && m_UseDate)
	{
		ASSERT(LFGetApp()->m_Attributes[p_VData->Attr].AttrProperties.Type==LFTypeTime);
		p_VData->IsNull = FALSE;

		SYSTEMTIME Date;
		m_wndCalendar.GetCurSel(&Date);

		if (m_UseTime)
		{
			SYSTEMTIME Time;
			m_wndTime.GetTime(&Time);

			Date.wHour = Time.wHour;
			Date.wMinute = Time.wMinute;
			Date.wSecond = Time.wSecond;
			Date.wMilliseconds = 0;
		}
		else
		{
			Date.wHour = Date.wMinute = Date.wSecond = Date.wMilliseconds = 0;
		}

		SYSTEMTIME stUTC;
		TzSpecificLocalTimeToSystemTime(NULL, &Date, &stUTC);
		SystemTimeToFileTime(&stUTC, &p_VData->Time);
	}
}

BOOL LFEditTimeDlg::InitDialog()
{
	if (m_lpszTemplateName==MAKEINTRESOURCE(IDD_EDITTIME))
		SetWindowText(LFGetApp()->GetAttributeName(p_VData->Attr));

	// Size
	CRect rect;
	m_wndCalendar.GetMinReqRect(&rect);

	CRect rectCalendar;
	m_wndCalendar.GetWindowRect(rectCalendar);

	INT GrowX = rect.Width()-rectCalendar.Width();
	INT GrowY = rect.Height()-rectCalendar.Height();

#define GrowXY(pWnd) { pWnd->GetWindowRect(rect); pWnd->SetWindowPos(NULL, 0, 0, rect.Width()+GrowX, rect.Height()+GrowY, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS); }
#define GrowX(pWnd) { pWnd->GetWindowRect(rect); pWnd->SetWindowPos(NULL, 0, 0, rect.Width()+GrowX, rect.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS); }
#define GrowXMoveY(pWnd) { pWnd->GetWindowRect(rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, rect.left, rect.top+GrowY, rect.Width()+GrowX, rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS); }

	GrowXY(this);
	GrowX(GetDlgItem(IDC_CATEGORY1));
	GrowXY(GetDlgItem(IDC_CALENDAR));
	GrowXMoveY(GetDlgItem(IDC_CATEGORY2));
	GrowXMoveY(GetDlgItem(IDC_USETIME));
	GrowXMoveY(GetDlgItem(IDC_TIME));

	// Data
	if (!p_VData->IsNull)
		if ((p_VData->Time.dwHighDateTime) || (p_VData->Time.dwLowDateTime))
		{
			SYSTEMTIME stUTC;
			SYSTEMTIME stLocal;
			FileTimeToSystemTime(&p_VData->Time, &stUTC);
			SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

			ENSURE(m_wndCalendar.SetCurSel(&stLocal));

			m_UseTime = (stLocal.wHour!=0) || (stLocal.wMinute!=0) || (stLocal.wSecond!=0);
			if (m_UseTime)
				ENSURE(m_wndTime.SetTime(&stLocal));
		}

	// Calendar
	m_wndCalendar.SetColor(MCSC_BACKGROUND, 0xFFFFFF);
	m_wndCalendar.SetColor(MCSC_MONTHBK, 0xFFFFFF);
	m_wndCalendar.SetColor(MCSC_TEXT, 0x000000);
	m_wndCalendar.SetColor(MCSC_TITLETEXT, IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOWTEXT));
	m_wndCalendar.SetColor(MCSC_TITLEBK, IsCtrlThemed() ? 0xCC3300 : GetSysColor(COLOR_WINDOW));
	m_wndCalendar.SetColor(MCSC_TRAILINGTEXT, 0xCCCCCC);

	SYSTEMTIME stMin;
	GetSystemTime(&stMin);
	stMin.wDay = 1;
	stMin.wMonth = 1;
	stMin.wYear = 1900;

	SYSTEMTIME stMax;
	GetSystemTime(&stMax);
	stMax.wDay = 31;
	stMax.wMonth = 12;
	stMax.wYear = 2099;

	ENSURE(m_wndCalendar.SetRange(&stMin, &stMax));

	// Time
	((CButton*)GetDlgItem(IDC_USETIME))->SetCheck(m_UseTime);
	EnableControls();

	return TRUE;
}

void LFEditTimeDlg::EnableControls()
{
	GetDlgItem(IDC_USETIME)->EnableWindow(m_UseDate);
	m_wndCalendar.EnableWindow(m_UseDate);
	m_wndTime.EnableWindow(m_UseDate && m_UseTime);
}


BEGIN_MESSAGE_MAP(LFEditTimeDlg, LFDialog)
	ON_BN_CLICKED(IDC_USETIME, OnUseTime)
END_MESSAGE_MAP()

void LFEditTimeDlg::OnUseTime()
{
	m_UseTime = ((CButton*)GetDlgItem(IDC_USETIME))->GetCheck();
	EnableControls();
}
