
// LFAboutDialog.cpp: Implementierung der Klasse LFAboutDialog
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <wininet.h>


// LFAboutDialog
//

#define COMPILE_HOUR       (((__TIME__[0]-'0')*10)+(__TIME__[1]-'0'))
#define COMPILE_MINUTE     (((__TIME__[3]-'0')*10)+(__TIME__[4]-'0'))
#define COMPILE_SECOND     (((__TIME__[6]-'0')*10)+(__TIME__[7]-'0'))
#define COMPILE_YEAR       ((((__DATE__[7]-'0')*10+(__DATE__[8]-'0'))*10+(__DATE__[9]-'0'))*10+(__DATE__[10]-'0'))
#define COMPILE_MONTH      ((__DATE__[2]=='n' ? (__DATE__[1] == 'a' ? 0 : 5)\
							: __DATE__[2]=='b' ? 1 \
							: __DATE__[2]=='r' ? (__DATE__[0] == 'M' ? 2 : 3) \
							: __DATE__[2]=='y' ? 4 \
							: __DATE__[2]=='l' ? 6 \
							: __DATE__[2]=='g' ? 7 \
							: __DATE__[2]=='p' ? 8 \
							: __DATE__[2]=='t' ? 9 \
							: __DATE__[2]=='v' ? 10 : 11)+1)
#define COMPILE_DAY        ((__DATE__[4]==' ' ? 0 : __DATE__[4]-'0')*10+(__DATE__[5]-'0'))

UINT LFAboutDialog::m_LastTab = 0;

LFAboutDialog::LFAboutDialog(USHORT BackgroundTabMask, CWnd* pParentWnd)
	: LFTabbedDialog(IDS_ABOUT, pParentWnd, &m_LastTab)
{
	m_BackgroundTabMask = BackgroundTabMask;

	// Compile time
	SYSTEMTIME st;
	ZeroMemory(&st, sizeof(st));

	st.wDay = COMPILE_DAY;
	st.wMonth = COMPILE_MONTH;
	st.wYear = COMPILE_YEAR;
	st.wHour = COMPILE_HOUR;
	st.wMinute = COMPILE_MINUTE;
	st.wSecond = COMPILE_SECOND;

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, m_BuildInfo, 256);
	wcscat_s(m_BuildInfo, 256, L", ");

	WCHAR tmpStr[256];
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOSECONDS, &st, NULL, tmpStr, 256);
	wcscat_s(m_BuildInfo, 256, tmpStr);

	// Special icons
	GetLocalTime(&st);

	p_AppLogo = LFGetApp()->GetCachedResourceImage((st.wMonth==3) && (st.wDay==17) ? IDB_STPATRICK : IDB_LIQUIDFOLDERS_128);
	p_SantaHat = (st.wMonth==12) ? LFGetApp()->GetCachedResourceImage(IDB_SANTA) : NULL;

	// Application name
	ENSURE(m_AppName.LoadString(IDR_APPLICATION));

	// Copyright
	GetFileVersion(AfxGetInstanceHandle(), m_Version, &m_Copyright);

	m_Copyright.Replace(_T(" liquidFOLDERS"), _T(""));
}

void LFAboutDialog::DoDataExchange(CDataExchange* pDX)
{
	LFTabbedDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);
	DDX_Control(pDX, IDC_ENABLEAUTOUPDATE, m_wndAutoUpdate);

	// Update settings
	BOOL EnableAutoUpdate;
	INT UpdateCheckInterval;

	if (pDX->m_bSaveAndValidate)
	{
		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		LFGetApp()->WriteUpdateSettings(EnableAutoUpdate, UpdateCheckInterval);
	}
	else
	{
		LFGetApp()->GetUpdateSettings(EnableAutoUpdate, UpdateCheckInterval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		OnEnableAutoUpdate();
	}
}

void LFAboutDialog::PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout)
{
	LFTabbedDialog::PaintOnBackground(dc, g, rectLayout);

	// Only draw custom background on "General" and "License" tab
	if (ShowBackgroundOnTab(m_CurrentTab))
	{
		// Icon
		g.DrawImage(p_AppLogo, rectLayout.left+m_ptAppLogo.x, rectLayout.top+m_ptAppLogo.y);

		if (p_SantaHat)
			g.DrawImage(p_SantaHat, rectLayout.left+m_ptAppLogo.x-39, rectLayout.top+m_ptAppLogo.y-8);

		// Caption
		CRect rectText(rectLayout);
		rectText.top += BACKSTAGEBORDER+128+2;

		CFont* pOldFont = dc.SelectObject(&m_CaptionFont);

		dc.SetTextColor(IsCtrlThemed() ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
		dc.DrawText(m_AppName, rectText, DT_SINGLELINE | DT_TOP | DT_CENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

		dc.SelectObject(pOldFont);
	}
}

BOOL LFAboutDialog::InitSidebar(LPSIZE pszTabArea)
{
	if (!LFTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_ABOUT_GENERAL, pszTabArea);
	AddTab(IDD_ABOUT_LICENSE, pszTabArea);

	return TRUE;
}

BOOL LFAboutDialog::InitDialog()
{
	// Version info
#ifdef _M_X64
	const UINT Bit = 64;
#else
	const UINT Bit = 32;
#endif

	CString Caption;
	m_wndVersionInfo.GetWindowText(Caption);

	CString Text;
	Text.Format(Caption, m_Version, Bit, m_Copyright, m_BuildInfo);

	m_wndVersionInfo.SetWindowText(Text);

	// Clone version info to other tabs with background
	ShowControlOnTabs(m_wndVersionInfo, m_BackgroundTabMask);

	// Background
	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(rectWnd);
	ScreenToClient(rectWnd);

	const INT WindowHeight = rectWnd.Height();

	const INT LineHeight = WindowHeight/24;
	const INT CaptionHeight = 4*LineHeight;
	const INT VersionHeight = 2*LineHeight;

	CRect rectLayout;
	GetLayoutRect(rectLayout);

	m_CaptionFont.CreateFont(CaptionHeight, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("DIN Mittelschrift"));
	m_VersionFont.CreateFont(VersionHeight);
	m_wndVersionInfo.SetFont(&m_VersionFont);

	m_ptAppLogo.x = rectWnd.left-rectLayout.left+(rectWnd.Width()-p_AppLogo->GetWidth())/2;
	m_ptAppLogo.y = BACKSTAGEBORDER;

	rectWnd.top = rectLayout.top+2*BACKSTAGEBORDER+128+CaptionHeight+4;
	m_wndVersionInfo.SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	// License
	CheckLicenseKey();

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return LFTabbedDialog::InitDialog();
}

void LFAboutDialog::CheckLicenseKey()
{
	LFLicense License;
	if (LFIsLicensed(&License))
	{
		// Set license text
		SetWindowTextA(GetDlgItem(IDC_REGNAME)->GetSafeHwnd(), License.RegName);
		SetWindowTextA(GetDlgItem(IDC_PURCHASEDATE)->GetSafeHwnd(), License.PurchaseDate);
		SetWindowTextA(GetDlgItem(IDC_PRODUCTID)->GetSafeHwnd(), License.ProductID);
		SetWindowTextA(GetDlgItem(IDC_QUANTITY)->GetSafeHwnd(), License.Quantity);

		// Remove button
		GetDlgItem(IDC_ENTERLICENSEKEY)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENTERLICENSEKEY)->SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);

		m_BackBufferL = m_BackBufferH = 0;
		Invalidate();
	}
}

void LFAboutDialog::CheckInternetConnection()
{
	DWORD Flags;
	GetDlgItem(IDC_UPDATENOW)->EnableWindow(InternetGetConnectedState(&Flags, 0));
}


BEGIN_MESSAGE_MAP(LFAboutDialog, LFTabbedDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()

	ON_NOTIFY(NM_CLICK, IDC_VERSIONINFO, OnVersionInfo)
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

void LFAboutDialog::OnDestroy()
{
	KillTimer(1);

	LFTabbedDialog::OnDestroy();
}

void LFAboutDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	LFTabbedDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void LFAboutDialog::OnVersionInfo(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	ShellExecute(GetSafeHwnd(), _T("open"), CString((LPCSTR)IDS_ABOUTURL), NULL, NULL, SW_SHOWNORMAL);

	*pResult = 0;
}

void LFAboutDialog::OnEnableAutoUpdate()
{
	const BOOL Enabled = m_wndAutoUpdate.GetCheck();

	GetDlgItem(IDC_CHECKDAILY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKWEEKLY)->EnableWindow(Enabled);
	GetDlgItem(IDC_CHECKMONTHLY)->EnableWindow(Enabled);
}

void LFAboutDialog::OnUpdateNow()
{
	LFGetApp()->CheckForUpdate(TRUE, this);
}

void LFAboutDialog::OnEnterLicenseKey()
{
	if (LFLicenseDlg(this).DoModal()==IDOK)
		CheckLicenseKey();
}
