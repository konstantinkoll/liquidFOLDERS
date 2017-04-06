
// LFEditTimeDlg.cpp: Implementierung der Klasse LFEditTimeDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditTimeDlg.h"


// LFEditTimeDlg
//

LFEditTimeDlg::LFEditTimeDlg(LFVariantData* pData, CWnd* pParentWnd)
	: LFDialog(IDD_EDITTIME, pParentWnd)
{
	ASSERT(pData);

	p_Data = pData;
	m_UseTime = TRUE;
}

void LFEditTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CALENDAR, m_wndCalendar);
	DDX_Control(pDX, IDC_TIME, m_wndTime);

	if (pDX->m_bSaveAndValidate)
	{
		ASSERT(LFGetApp()->m_Attributes[p_Data->Attr].AttrProperties.Type==LFTypeTime);
		p_Data->IsNull = FALSE;

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
		SystemTimeToFileTime(&stUTC, &p_Data->Time);
	}
}

BOOL LFEditTimeDlg::InitDialog()
{
	SetWindowText(LFGetApp()->m_Attributes[p_Data->Attr].Name);

	// Gr��e
	CRect rect;
	m_wndCalendar.GetMinReqRect(&rect);

	CRect rectCalendar;
	m_wndCalendar.GetWindowRect(rectCalendar);

	INT GrowX = rect.Width()-rectCalendar.Width();
	INT GrowY = rect.Height()-rectCalendar.Height();

#define GrowXY(pWnd) { pWnd->GetWindowRect(rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, 0, 0, rect.Width()+GrowX, rect.Height()+GrowY, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS); }
#define GrowX(pWnd) { pWnd->GetWindowRect(rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, 0, 0, rect.Width()+GrowX, rect.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS); }
#define GrowXMoveY(pWnd) { pWnd->GetWindowRect(rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, rect.left, rect.top+GrowY, rect.Width()+GrowX, rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS); }

	GrowXY(this);
	GrowX(GetDlgItem(IDC_CATEGORY1));
	GrowXY(GetDlgItem(IDC_CALENDAR));
	GrowXMoveY(GetDlgItem(IDC_CATEGORY2));
	GrowXMoveY(GetDlgItem(IDC_USETIME));
	GrowXMoveY(GetDlgItem(IDC_TIME));

	// Werte
	if (!p_Data->IsNull)
		if ((p_Data->Time.dwHighDateTime) || (p_Data->Time.dwLowDateTime))
		{
			SYSTEMTIME stUTC;
			SYSTEMTIME stLocal;
			FileTimeToSystemTime(&p_Data->Time, &stUTC);
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
	m_wndTime.EnableWindow(m_UseTime);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFEditTimeDlg, LFDialog)
	ON_BN_CLICKED(IDC_USETIME, OnUseTime)
END_MESSAGE_MAP()

void LFEditTimeDlg::OnUseTime()
{
	m_UseTime = ((CButton*)GetDlgItem(IDC_USETIME))->GetCheck();
	m_wndTime.EnableWindow(m_UseTime);
}
