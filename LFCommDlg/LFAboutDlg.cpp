
// LFAboutDlg.cpp: Implementierung der Klasse LFAboutDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <wininet.h>


// LFAboutDlg
//

#define COMPILE_HOUR       (((__TIME__[0]-'0')*10)+(__TIME__[1]-'0'))
#define COMPILE_MINUTE     (((__TIME__[3]-'0')*10)+(__TIME__[4]-'0'))
#define COMPILE_SECOND     (((__TIME__[6]-'0')*10)+(__TIME__[7]-'0'))
#define COMPILE_YEAR       ((((__DATE__[7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__[9]-'0'))*10+(__DATE__[10]-'0'))
#define COMPILE_MONTH      ((__DATE__[2]=='n' ? (__DATE__ [1] == 'a' ? 0 : 5)\
							: __DATE__[2]=='b' ? 1 \
							: __DATE__[2]=='r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
							: __DATE__[2]=='y' ? 4 \
							: __DATE__[2]=='l' ? 6 \
							: __DATE__[2]=='g' ? 7 \
							: __DATE__[2]=='p' ? 8 \
							: __DATE__[2]=='t' ? 9 \
							: __DATE__[2]=='v' ? 10 : 11)+1)
#define COMPILE_DAY        ((__DATE__[4]==' ' ? 0 : __DATE__[4]-'0')*10+(__DATE__[5]-'0'))

LFAboutDlg::LFAboutDlg(CWnd* pParentWnd)
	: LFDialog(IDD_ABOUT, pParentWnd)
{
	m_CaptionTop = m_IconTop = 0;

	SYSTEMTIME st;
	ZeroMemory(&st, sizeof(st));
	st.wDay = COMPILE_DAY;
	st.wMonth = COMPILE_MONTH;
	st.wYear = COMPILE_YEAR;
	st.wHour = COMPILE_HOUR;
	st.wMinute = COMPILE_MINUTE;
	st.wSecond = COMPILE_SECOND;

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, m_Build, 256);
	wcscat_s(m_Build, 256, L", ");

	WCHAR tmpStr[256];
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOSECONDS, &st, NULL, tmpStr, 256);
	wcscat_s(m_Build, 256, tmpStr);

	GetSystemTime(&st);
	p_Santa = (st.wMonth==12) ? LFGetApp()->GetCachedResourceImage(IDB_SANTA, _T("PNG")) : NULL;
	p_Logo = LFGetApp()->GetCachedResourceImage(IDB_LIQUIDFOLDERS_128, _T("PNG"));

	GetFileVersion(AfxGetInstanceHandle(), &m_Version, &m_Copyright);
	m_Copyright.Replace(_T(" liquidFOLDERS"), _T(""));

	ENSURE(m_AppName.LoadString(IDR_APPLICATION));
}

void LFAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);

	BOOL EnableAutoUpdate;
	INT UpdateCheckInterval;

	if (pDX->m_bSaveAndValidate)
	{
		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		LFGetApp()->SetUpdateSettings(EnableAutoUpdate, UpdateCheckInterval);
	}
	else
	{
		LFGetApp()->GetUpdateSettings(&EnableAutoUpdate, &UpdateCheckInterval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		OnEnableAutoUpdate();
	}
}

void LFAboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	g.DrawImage(p_Logo->m_pBitmap, p_Santa ? 39 : 9, m_IconTop);
	if (p_Santa)
		g.DrawImage(p_Santa->m_pBitmap, -6, m_IconTop-10);

	CRect r(rect);
	r.top = m_CaptionTop;
	r.left = (p_Santa ? 178 : 148)-1;

	CFont* pOldFont = dc.SelectObject(&m_CaptionFont);

	const UINT fmt = DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS;
	dc.SetTextColor(IsCtrlThemed() ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_AppName, r, fmt);

	dc.SelectObject(pOldFont);
}

void LFAboutDlg::CheckLicenseKey()
{
	LFLicense License;
	if (LFIsLicensed(&License))
	{
		GetDlgItem(IDC_ENTERLICENSEKEY)->ShowWindow(SW_HIDE);
		Invalidate();
	}

	SetWindowTextA(GetDlgItem(IDC_REGNAME)->GetSafeHwnd(), License.RegName);
	SetWindowTextA(GetDlgItem(IDC_PURCHASEDATE)->GetSafeHwnd(), License.PurchaseDate);
	SetWindowTextA(GetDlgItem(IDC_PURCHASEID)->GetSafeHwnd(), License.PurchaseID);
	SetWindowTextA(GetDlgItem(IDC_PRODUCT)->GetSafeHwnd(), License.ProductID);

	if (strlen(License.ProductID)>13)
	{
		GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_SHOW);
		SetWindowTextA(GetDlgItem(IDC_QUANTITY)->GetSafeHwnd(), License.Quantity);
	}
}

void LFAboutDlg::CheckInternetConnection()
{
	DWORD Flags;
	GetDlgItem(IDC_UPDATENOW)->EnableWindow(InternetGetConnectedState(&Flags, 0));
}


BEGIN_MESSAGE_MAP(LFAboutDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
	ON_NOTIFY(NM_CLICK, IDC_VERSIONINFO, OnVersionInfo)
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

BOOL LFAboutDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Version
	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(&rectWnd);
	ScreenToClient(&rectWnd);

#ifdef _M_X64
#define ISET _T(" (x64)")
#else
#define ISET _T(" (x86)")
#endif

	CString Caption;
	m_wndVersionInfo.GetWindowText(Caption);
	CString text;
	text.Format(Caption, m_Version+ISET, m_Build, m_Copyright);
	m_wndVersionInfo.SetWindowText(text);

	// Hintergrund
	const INT Height = rectWnd.Height()-16;
	const INT LineGap = Height/11;
	const INT HeightCaption = 4*LineGap;
	const INT HeightVersion = 2*LineGap;

	m_CaptionFont.CreateFont(HeightCaption, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Letter Gothic"));

	m_VersionFont.CreateFont(HeightVersion, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, LFGetApp()->GetDefaultFontFace());
	m_wndVersionInfo.SetFont(&m_VersionFont);

	m_CaptionTop = rectWnd.top+(rectWnd.bottom-HeightCaption-LineGap-3*HeightVersion)/2-8;
	m_IconTop = rectWnd.top+(rectWnd.bottom-124)/2-8;

	rectWnd.left = p_Santa ? 178 : 148;
	rectWnd.top = m_CaptionTop+HeightCaption+LineGap;
	m_wndVersionInfo.SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	// Lizenz
	CheckLicenseKey();

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return FALSE;
}

void LFAboutDlg::OnDestroy()
{
	KillTimer(1);

	LFDialog::OnDestroy();
}

void LFAboutDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	LFDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void LFAboutDlg::OnEnableAutoUpdate()
{
	BOOL Enabled = ((CButton*)GetDlgItem(IDC_ENABLEAUTOUPDATE))->GetCheck();

	GetDlgItem(IDC_CHECKDAILY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKWEEKLY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKMONTHLY)->EnableWindow(Enabled);
}

void LFAboutDlg::OnUpdateNow()
{
	LFCheckForUpdate(TRUE, this);
}

void LFAboutDlg::OnVersionInfo(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	CString URL((LPCSTR)IDS_ABOUTURL);

	ShellExecute(GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOW);

	*pResult = 0;
}

void LFAboutDlg::OnEnterLicenseKey()
{
	LFLicenseDlg dlg(this);
	dlg.DoModal();

	CheckLicenseKey();
}
