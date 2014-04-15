
// LFAboutDlg.cpp: Implementierung der Klasse LFAboutDlg
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include "Resource.h"
#include <wininet.h>


// LFAboutDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFAboutDlg::LFAboutDlg(CString Build, CWnd* pParentWnd)
	: LFDialog(IDD_ABOUT, pParentWnd)
{
	m_Build = Build;
	m_CaptionTop = m_IconTop = 0;

	SYSTEMTIME st;
	GetSystemTime(&st);
	p_Santa = (st.wMonth==12) ? p_App->GetCachedResourceImage(IDB_SANTA, _T("PNG"), LFCommDlgDLL.hResource) : NULL;
	p_Logo = p_App->GetCachedResourceImage(IDB_LIQUIDFOLDERS_128, _T("PNG"), LFCommDlgDLL.hResource);

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

		p_App->SetUpdateSettings(EnableAutoUpdate, UpdateCheckInterval);
	}
	else
	{
		p_App->GetUpdateSettings(&EnableAutoUpdate, &UpdateCheckInterval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, UpdateCheckInterval);

		OnEnableAutoUpdate();
	}
}

void LFAboutDlg::CheckLicenseKey()
{
	LFLicense l;
	if (LFIsLicensed(&l))
		GetDlgItem(IDC_ENTERLICENSEKEY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_NAME)->SetWindowText(l.RegName);
	GetDlgItem(IDC_PURCHASEDATE)->SetWindowText(l.PurchaseDate);
	GetDlgItem(IDC_ID)->SetWindowText(l.PurchaseID);
	GetDlgItem(IDC_PRODUCT)->SetWindowText(l.ProductID);

	if (wcslen(l.ProductID)>13)
	{
		GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUANTITY)->SetWindowText(l.Quantity);
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

	CString caption;
	m_wndVersionInfo.GetWindowText(caption);
	CString text;
	text.Format(caption, m_Version+ISET, m_Build, m_Copyright);
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
		DEFAULT_PITCH | FF_DONTCARE, p_App->GetDefaultFontFace());
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

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
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
	dc.SetTextColor(IsCtrlThemed() ? 0xCB3300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_AppName, r, fmt);

	dc.SelectObject(pOldFont);
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
	CString url;
	ENSURE(url.LoadString(IDS_ABOUTURL));

	ShellExecute(GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);

	*pResult = 0;
}

void LFAboutDlg::OnEnterLicenseKey()
{
	LFLicenseDlg dlg(this);
	dlg.DoModal();

	CheckLicenseKey();
}
