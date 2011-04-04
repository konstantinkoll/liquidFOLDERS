
// LFAboutDlg.cpp: Implementierung der Klasse LFAboutDlg
//

#include "stdafx.h"
#include "LFAboutDlg.h"
#include "LFCore.h"
#include "Resource.h"
#include "..\\LFCore\\resource.h"

using namespace Gdiplus;


// LFAboutDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFAboutDlg::LFAboutDlg(CString AppName, CString Build, UINT IconResID, CWnd* pParent)
	: LFDialog(IDD_ABOUT, LFDS_Default, pParent)
{
	m_AppName = AppName;
	m_Build = Build;
	ENSURE(m_Icon.Load(IconResID, _T("PNG"), AfxGetResourceHandle()));

	CString modFilename;
	if (GetModuleFileName(AfxGetInstanceHandle(), modFilename.GetBuffer(MAX_PATH), MAX_PATH)>0)
	{
		modFilename.ReleaseBuffer(MAX_PATH);
		DWORD dwHandle = 0;
		DWORD dwSize = GetFileVersionInfoSize(modFilename, &dwHandle);
		if (dwSize>0)
		{
			LPBYTE lpInfo = new BYTE[dwSize];
			ZeroMemory(lpInfo, dwSize);

			if (GetFileVersionInfo(modFilename, 0, dwSize, lpInfo))
			{
				UINT valLen = MAX_PATH;
				LPVOID valPtr = NULL;
				LPCWSTR valData = NULL;

				if (VerQueryValue(lpInfo, _T("\\"), &valPtr, &valLen))
				{
					VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;
					m_Version.Format(_T("%d.%d.%d"), 
						(pFinfo->dwProductVersionMS >> 16) & 0xFF,
						(pFinfo->dwProductVersionMS) & 0xFF,
						(pFinfo->dwProductVersionLS >> 16) & 0xFF);
				}

				m_Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("© liquidFOLDERS");
			}
			delete[] lpInfo;
		}
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

	g.DrawImage(m_Icon.m_pBitmap, 16, 16, m_Icon.m_pBitmap->GetWidth(), m_Icon.m_pBitmap->GetHeight());

	CRect r(rect);
	r.top = 172;
	r.left = 16;

	CFont font1;
	font1.CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, ((LFApplication*)AfxGetApp())->GetDefaultFontFace());
	CFont* pOldFont = dc.SelectObject(&font1);

	dc.SetTextColor(0x000000);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(m_AppName+_T(" (Beta5)"), r, 0);
	r.top += 45;

	CFont font2;
	font2.CreateFont(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, ((LFApplication*)AfxGetApp())->GetDefaultFontFace());
	dc.SelectObject(&font2);

	dc.DrawText(_T("Version ")+m_Version+_T(" (")+m_Build+_T(")"), r, 0);
	r.top += 25;

	dc.DrawText(m_Copyright, r, 0);

	dc.SelectObject(pOldFont);
}
