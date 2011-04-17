
// LFEditTimeDlg.cpp: Implementierung der Klasse LFEditTimeDlg
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include "Resource.h"


// LFEditTimeDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFEditTimeDlg::LFEditTimeDlg(CWnd* pParentWnd, UINT Attr)
	: CDialog(IDD_EDITTIME, pParentWnd)
{
	p_App = (LFApplication*)AfxGetApp();
	m_Attr = Attr;
}

void LFEditTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_CALENDAR, m_wndCalendar);

	if (pDX->m_bSaveAndValidate)
	{
	}
}


BEGIN_MESSAGE_MAP(LFEditTimeDlg, CDialog)
END_MESSAGE_MAP()

BOOL LFEditTimeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(p_App->m_Attributes[m_Attr]->Name);

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	INT idx = GetAttributeIconIndex(m_Attr);
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

	return TRUE;
}
