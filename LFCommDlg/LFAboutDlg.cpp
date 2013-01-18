
// LFAboutDlg.cpp: Implementierung der Klasse LFAboutDlg
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include "Resource.h"


// LFAboutDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFAboutDlg::LFAboutDlg(CString AppName, CString Build, UINT IconResID, CWnd* pParent)
	: LFDialog(IDD_ABOUT, LFDS_Default, pParent)
{
	m_AppName = AppName;
	m_Build = Build;
	ENSURE(m_Icon.Load(IconResID, _T("PNG"), AfxGetResourceHandle()));

	GetFileVersion(AfxGetInstanceHandle(), &m_Version, &m_Copyright);
}

void LFAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	BOOL EnableAutoUpdate;
	INT Interval;

	if (pDX->m_bSaveAndValidate)
	{
		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, Interval);

		LFGetApp()->SetUpdateSettings(EnableAutoUpdate, Interval);
	}
	else
	{
		LFGetApp()->GetUpdateSettings(&EnableAutoUpdate, &Interval);

		DDX_Check(pDX, IDC_ENABLEAUTOUPDATE, EnableAutoUpdate);
		DDX_Radio(pDX, IDC_CHECKDAILY, Interval);

		OnEnableAutoUpdate();
	}
}

void LFAboutDlg::CheckLicenseKey(LFLicense* License)
{
	LFLicense l;
	if (!License)
		License = &l;

	LFDialog::CheckLicenseKey(License);

	GetDlgItem(IDC_NAME)->SetWindowText(License->RegName);
	GetDlgItem(IDC_PURCHASEDATE)->SetWindowText(License->PurchaseDate);
	GetDlgItem(IDC_ID)->SetWindowText(License->PurchaseID);
	GetDlgItem(IDC_PRODUCT)->SetWindowText(License->ProductID);

	if (wcslen(License->ProductID)>13)
	{
		GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_QUANTITYTITLE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_QUANTITY)->SetWindowText(License->Quantity);
	}
}


BEGIN_MESSAGE_MAP(LFAboutDlg, LFDialog)
	ON_BN_CLICKED(IDC_ENABLEAUTOUPDATE, OnEnableAutoUpdate)
	ON_BN_CLICKED(IDC_UPDATENOW, OnUpdateNow)
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

	// Lizenz
	CheckLicenseKey();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFAboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	g.DrawImage(m_Icon.m_pBitmap, 9, 14, m_Icon.m_pBitmap->GetWidth(), m_Icon.m_pBitmap->GetHeight());

	CRect r(rect);
	r.top = 15;
	r.left = 148;

	CFont font1;
	font1.CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, LFGetApp()->GetDefaultFontFace());
	CFont* pOldFont = dc.SelectObject(&font1);

	const UINT fmt = DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS;
	dc.SetTextColor(0x000000);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_AppName, r, fmt);
	r.top += 45;

	CFont font2;
	font2.CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, LFGetApp()->GetDefaultFontFace());
	dc.SelectObject(&font2);

#ifdef _M_X64
#define ISET _T(" (x64)")
#else
#define ISET _T(" (x32)")
#endif

	dc.DrawText(_T("Version ")+m_Version+ISET, r, fmt);
	r.top += 25;

	dc.DrawText(_T("Built ")+m_Build, r, fmt);
	r.top += 25;

	dc.DrawText(m_Copyright, r, fmt);

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
