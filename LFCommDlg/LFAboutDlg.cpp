
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

LFAboutDlg::LFAboutDlg(CString AppName, CString Build, UINT IconResID, CWnd* pParentWnd)
	: LFDialog(IDD_ABOUT, pParentWnd)
{
	m_AppName = AppName;
	m_Build = Build;
	m_CaptionTop = 0;

	ENSURE(m_Logo.Load(IconResID, _T("PNG")));

	SYSTEMTIME st;
	GetSystemTime(&st);
	m_pSanta = (st.wMonth==12) ? new CGdiPlusBitmapResource(IDB_SANTA, _T("PNG"), LFCommDlgDLL.hResource) : NULL;

	GetFileVersion(AfxGetInstanceHandle(), &m_Version, &m_Copyright);
	m_Copyright.Replace(_T(" liquidFOLDERS"), _T(""));
}

void LFAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);

	BOOL EnableAutoUpdate;
	INT Interval;

	if (pDX->m_bSaveAndValidate)
	{
		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, Interval);

		p_App->SetUpdateSettings(EnableAutoUpdate, Interval);
	}
	else
	{
		p_App->GetUpdateSettings(&EnableAutoUpdate, &Interval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, Interval);

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

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, m_AppName);
	SetWindowText(caption);

	// Version
#ifdef _M_X64
#define ISET _T(" (x64)")
#else
#define ISET _T(" (x86)")
#endif

	m_wndVersionInfo.GetWindowText(caption);
	text.Format(caption, m_Version+ISET, m_Build, m_Copyright);
	m_wndVersionInfo.SetWindowText(text);

	// Hintergrund
	m_CaptionFont.CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Letter Gothic"));

	m_VersionFont.CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, p_App->GetDefaultFontFace());
	m_wndVersionInfo.SetFont(&m_VersionFont);

	CDC* dc = GetDC();
	CFont* pOldFont = dc->SelectObject(&m_CaptionFont);
	INT HeightCaption = dc->GetTextExtent(_T("Wy")).cy+8;
	dc->SelectObject(&m_VersionFont);
	INT HeightVersion = dc->GetTextExtent(_T("Wy")).cy*3;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_CaptionTop = (128+(p_App->OSVersion==OS_XP ? 20 : 12)-HeightCaption-HeightVersion)/2;

	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(&rectWnd);
	ScreenToClient(&rectWnd);
	rectWnd.left = m_pSanta ? 178 : 148;
	rectWnd.top = m_CaptionTop+HeightCaption;
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

	if (m_pSanta)
		delete m_pSanta;

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

	g.DrawImage(m_Logo.m_pBitmap, m_pSanta ? 39 : 9, 12, 128, 128);
	if (m_pSanta)
		g.DrawImage(m_pSanta->m_pBitmap, -6, 2);

	CRect r(rect);
	r.top = m_CaptionTop;
	r.left = m_pSanta ? 178 : 148;

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
