#include "stdafx.h"
#include "LFAboutDlg.h"
#include "LFCore.h"
#include "Resource.h"
#include "..\\LFCore\\resource.h"

using namespace Gdiplus;

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFAboutDlg::LFAboutDlg(LFAboutDlgParameters* pParameters, CWnd* pParent)
	: LFDialog(IDD_ABOUT, pParent)
{
	ASSERT(pParameters!=NULL);
	parameters = pParameters;
	parameters->caption.Remove('&');
	parameters->caption.Remove('.');

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
					parameters->version.Format(_T("%d.%d.%d"), 
						(pFinfo->dwProductVersionMS >> 16) & 0xFF,
						(pFinfo->dwProductVersionMS) & 0xFF,
						(pFinfo->dwProductVersionLS >> 16) & 0xFF);
				}
				if (VerQueryValue(lpInfo,TEXT("StringFileInfo\\000004E4\\LegalCopyright"),(void**)&valData,&valLen))
				{
					parameters->copyright=valData;
				}
				else
				{
					parameters->copyright="© liquidFOLDERS";
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
	SetWindowText(parameters->caption);

	BOOL ShowCancel = FALSE;

	// Combobox für das Farbschema füllen
	CComboBox* cbx = (CComboBox*)GetDlgItem(IDC_RIBBONCOLORCOMBO);
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_RIBBONCOLORITEMS));
	int y;
	do
	{
		y = tmpStr.Find(';');
		if (y>=0)
		{
			cbx->AddString(tmpStr.Left(y));
			tmpStr = tmpStr.Right(tmpStr.GetLength()-y-1);
		} 
	}
	while (y>=0);
	cbx->AddString(tmpStr);

	// Combobox einstellen
	if (parameters->RibbonColor==ID_VIEW_APPLOOK_OFF_2007_NONE)
	{
		cbx->EnableWindow(FALSE);
	}
	else
	{
		cbx->SetCurSel(parameters->RibbonColor-1);
		ShowCancel = TRUE;
	}

	// Radiobuttons für die Texturgröße einstellen
	BOOL bEnable[5];

	for (int a=LFTextureAuto; a<=LFTexture8192; a++)
	{
		bEnable[a] = (parameters->TextureSize!=LFTextureNone) && ((a<=LFTexture1024) || (parameters->MaxTextureSize>=a));
		((CButton*)GetDlgItem(IDC_TEXTURE_AUTO+a))->EnableWindow(bEnable[a]);
	}

	if (parameters->TextureSize!=-1)
	{
		if ((!bEnable[parameters->TextureSize]) || (parameters->TextureSize>LFTexture8192))
			parameters->TextureSize = LFTextureAuto;
		((CButton*)GetDlgItem(IDC_TEXTURE_AUTO+parameters->TextureSize))->SetCheck(TRUE);
		ShowCancel = TRUE;
	}

	// Radiobuttons für Laufwerke einstellen
	CButton* HideEmpty = ((CButton*)GetDlgItem(IDC_DRIVES_HIDEEMPTY));
	CButton* ShowAll = ((CButton*)GetDlgItem(IDC_DRIVES_SHOWALL));

	if (parameters->AllowEmptyDrives==-1)
	{
		HideEmpty->EnableWindow(FALSE);
		ShowAll->EnableWindow(FALSE);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_DRIVES_HIDEEMPTY+parameters->AllowEmptyDrives))->SetCheck(TRUE);
		ShowCancel = TRUE;
	}

	// Ggf. "Abbrechen" verschwinden lassen
	if (!ShowCancel)
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

	CheckLicenseKey();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFAboutDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	if (parameters->icon)
		g.DrawImage(parameters->icon->m_pBitmap, 16, 16, parameters->icon->m_pBitmap->GetWidth(), parameters->icon->m_pBitmap->GetHeight());

	CRect r = rect;
	r.top = 32+logo->m_pBitmap->GetHeight();
	r.left = 16;

	HFONT font = CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));
	HGDIOBJ oldFont =dc.SelectObject(font);

	dc.SetTextColor(0x000000);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawTextW(parameters->appname, -1, r, 0);
	r.top += 45;

	DeleteObject(font);

	font = CreateFont(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));
	dc.SelectObject(font);

	dc.DrawTextW(parameters->copyright, -1, r, 0);
	r.top += 25;

	dc.DrawTextW(_T("Version ")+parameters->version+_T(" (")+parameters->build+_T(")"), -1, r, 0);
	r.top += 25;

	dc.SelectObject(oldFont);
	DeleteObject(font);
}

void LFAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDOK, m_OkButton);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_ENTERLICENSEKEY, m_LicenseButton);

	if (pDX->m_bSaveAndValidate)
	{
		for (UINT a=LFTextureAuto; a<=LFTexture8192; a++)
			if (((CButton*)GetDlgItem(IDC_TEXTURE_AUTO+a))->GetCheck())
			{
				parameters->TextureSize = a;
				break;
			}

		int Look = ((CComboBox*)GetDlgItem(IDC_RIBBONCOLORCOMBO))->GetCurSel()+1;
		if ((Look!=parameters->RibbonColor) && (parameters->RibbonColor!=ID_VIEW_APPLOOK_OFF_2007_NONE))
		{
			parameters->RibbonColor = Look;
			::SendNotifyMessage(HWND_BROADCAST, LFGetMessageIDs()->LookChanged, Look, 0);
		}

		parameters->AllowEmptyDrives = ((CButton*)GetDlgItem(IDC_DRIVES_SHOWALL))->GetCheck();
	}
}
