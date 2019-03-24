
// LFGenericCloudProviderDlg.cpp: Implementierung der Klasse LFGenericCloudProviderDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFGenericCloudProviderDlg
//

LFGenericCloudProviderDlg::LFGenericCloudProviderDlg(const CString& ProviderName, LPCWSTR lpcPath, CWnd* pParentWnd)
	: LFDialog(IDD_GENERICCLOUDPROVIDER, pParentWnd)
{
	assert(lpcPath);

	m_ProviderName = ProviderName;
	wcscpy_s(m_FolderPath, MAX_PATH, lpcPath);
}

void LFGenericCloudProviderDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ITEMPANEL, m_wndPanel);
}

BOOL LFGenericCloudProviderDlg::InitDialog()
{
	// Item panel
	m_wndPanel.SetItem(m_FolderPath);

	// Caption
	SetWindowText(m_ProviderName);

	// Disable button
	GetDlgItem(IDOK)->EnableWindow(m_FolderPath[0]!=L'\0');

	return TRUE;
}
