
// LFEditTimeDlg.cpp: Implementierung der Klasse LFEditTimeDlg
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include "Resource.h"


// LFEditTimeDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFEditTimeDlg::LFEditTimeDlg(CWnd* pParentWnd, LFVariantData* pData)
	: CDialog(IDD_EDITTIME, pParentWnd)
{
	ASSERT(pData);

	p_App = (LFApplication*)AfxGetApp();
	p_Data = pData;
	m_UseTime = TRUE;
}

void LFEditTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_CALENDAR, m_wndCalendar);
	DDX_Control(pDX, IDC_TIME, m_wndTime);

	if (pDX->m_bSaveAndValidate)
	{
		ASSERT(p_App->m_Attributes[p_Data->Attr]->Type==LFTypeTime);
		p_Data->IsNull = false;

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


BEGIN_MESSAGE_MAP(LFEditTimeDlg, CDialog)
	ON_BN_CLICKED(IDC_USETIME, OnUseTime)
END_MESSAGE_MAP()

BOOL LFEditTimeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(p_App->m_Attributes[p_Data->Attr]->Name);

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	INT idx = GetAttributeIconIndex(p_Data->Attr);
	if (idx!=-1)
	{
		CImageListTransparent AttributeIcons;
		AttributeIcons.Create(IDB_ATTRIBUTEICONS_16, LFCommDlgDLL.hResource);

		HICON hIcon = AttributeIcons.ExtractIcon(idx);
		SetIcon(hIcon, FALSE);
		SetIcon(hIcon, TRUE);
	}

	// Größe
	CRect rect;
	m_wndCalendar.GetMinReqRect(&rect);

	CRect rectCalendar;
	m_wndCalendar.GetWindowRect(&rectCalendar);

	INT GrowX = rect.Width()-rectCalendar.Width()+4;
	INT GrowY = rect.Height()-rectCalendar.Height()+4;

#define Grow(pWnd) { pWnd->GetWindowRect(&rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, 0, 0, rect.Width()+GrowX, rect.Height()+GrowY, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE); }
#define GXMY(pWnd) { pWnd->GetWindowRect(&rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, rect.left, rect.top+GrowY, rect.Width()+GrowX, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE); }
#define Move(pWnd) { pWnd->GetWindowRect(&rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, rect.left+GrowX, rect.top+GrowY, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE); }

	Grow(this);
	Grow(GetDlgItem(IDC_GROUPBOX1));
	Grow(GetDlgItem(IDC_CALENDAR));
	GXMY(GetDlgItem(IDC_GROUPBOX2));
	GXMY(GetDlgItem(IDC_USETIME));
	GXMY(GetDlgItem(IDC_TIME));
	Move(GetDlgItem(IDOK));
	Move(GetDlgItem(IDCANCEL));

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
	m_wndCalendar.GetWindowRect(&rectCalendar);
	ScreenToClient(rectCalendar);
	m_FrameCtrl.Create(this, rectCalendar);

	rectCalendar.DeflateRect(2, 2);
	m_wndCalendar.SetWindowPos(NULL, rectCalendar.left, rectCalendar.top, rectCalendar.Width(), rectCalendar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	m_wndCalendar.SetColor(MCSC_BACKGROUND, GetSysColor(COLOR_WINDOW));
	m_wndCalendar.SetColor(MCSC_MONTHBK, GetSysColor(COLOR_WINDOW));
	m_wndCalendar.SetColor(MCSC_TEXT, GetSysColor(COLOR_WINDOWTEXT));
	m_wndCalendar.SetColor(MCSC_TITLETEXT, 0xFFFFFF);
	m_wndCalendar.SetColor(MCSC_TITLEBK, 0x993300);
	m_wndCalendar.SetColor(MCSC_TRAILINGTEXT, GetSysColor(COLOR_3DSHADOW));

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

void LFEditTimeDlg::OnUseTime()
{
	m_UseTime = ((CButton*)GetDlgItem(IDC_USETIME))->GetCheck();
	m_wndTime.EnableWindow(m_UseTime);
}
