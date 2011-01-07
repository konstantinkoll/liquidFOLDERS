
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

LFAboutDlg::LFAboutDlg(LFAboutDlgParameters* p, CWnd* pParent)
	: LFDialog(IDD_ABOUT, LFDS_Default, pParent)
{
	ASSERT(pp_Parameters!=NULL);
	p_Parameters = p;

	CString modFilename;
	if (GetModuleFileName(AfxGetInstanceHandle(), modFilename.GetBuffer(MAX_PATH), MAX_PATH) > 0)
	{
		modFilename.ReleaseBuffer(MAX_PATH);
		DWORD dwHandle = 0;
		DWORD dwSize = GetFileVersionInfoSize(modFilename, &dwHandle);
		if (dwSize > 0)
		{
			LPBYTE lpInfo = new BYTE[dwSize];
			ZeroMemory(lpInfo, dwSize);
			if (GetFileVersionInfo(modFilename, 0, dwSize, lpInfo))
			{
				UINT valLen = MAX_PATH;
				LPVOID valPtr = NULL;
				LPCWSTR valData = NULL;
				if (VerQueryValue(lpInfo, TEXT("\\"), &valPtr, &valLen))
				{
					VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;
					p_Parameters->Version.Format(_T("%d.%d.%d"), 
						(pFinfo->dwProductVersionMS >> 16) & 0xFF,
						(pFinfo->dwProductVersionMS) & 0xFF,
						(pFinfo->dwProductVersionLS >> 16) & 0xFF);
				}
				if (VerQueryValue(lpInfo,TEXT("StringFileInfo\\000004E4\\LegalCopyright"),(void**)&valData,&valLen))
				{
					p_Parameters->Copyright = valData;
				}
				else
				{
					p_Parameters->Copyright="© liquidFOLDERS";
				}
			}
			delete[] lpInfo;
		}
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
	caption.Format(text, p_Parameters->AppName);
	SetWindowText(caption);

	BOOL ShowCancel = FALSE;

	// Radiobuttons für die Texturgröße einstellen
	BOOL bEnable[5];

	for (INT a=LFTextureAuto; a<=LFTexture8192; a++)
	{
		bEnable[a] = (p_Parameters->TextureSize!=LFTextureNone) && ((a<=LFTexture1024) || (p_Parameters->MaxTextureSize>=a));
		((CButton*)GetDlgItem(IDC_TEXTURE_AUTO+a))->EnableWindow(bEnable[a]);
	}

	if (p_Parameters->TextureSize!=-1)
	{
		if ((!bEnable[p_Parameters->TextureSize]) || (p_Parameters->TextureSize>LFTexture8192))
			p_Parameters->TextureSize = LFTextureAuto;
		((CButton*)GetDlgItem(IDC_TEXTURE_AUTO+p_Parameters->TextureSize))->SetCheck(TRUE);
		ShowCancel = TRUE;
	}

	// Ggf. "Abbrechen" verschwinden lassen
	if (!ShowCancel)
	{
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

		CRect rect;
		GetDlgItem(IDCANCEL)->GetWindowRect(rect);
		ScreenToClient(&rect);

		GetDlgItem(IDOK)->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	// Lizenz
	CheckLicenseKey();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFAboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	if (p_Parameters->Icon)
		g.DrawImage(p_Parameters->Icon->m_pBitmap, 16, 16, p_Parameters->Icon->m_pBitmap->GetWidth(), p_Parameters->Icon->m_pBitmap->GetHeight());

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
	dc.DrawText(p_Parameters->AppName+_T(" (Beta3)"), -1, r, 0);
	r.top += 45;

	CFont font2;
	font2.CreateFont(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, ((LFApplication*)AfxGetApp())->GetDefaultFontFace());
	dc.SelectObject(&font2);

	dc.DrawText(p_Parameters->Copyright, -1, r, 0);
	r.top += 25;

	dc.DrawText(_T("Version ")+p_Parameters->Version+_T(" (")+p_Parameters->Build+_T(")"), -1, r, 0);

	dc.SelectObject(pOldFont);
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

void LFAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
		for (UINT a=LFTextureAuto; a<=LFTexture8192; a++)
			if (((CButton*)GetDlgItem(IDC_TEXTURE_AUTO+a))->GetCheck())
			{
				p_Parameters->TextureSize = a;
				break;
			}
}
