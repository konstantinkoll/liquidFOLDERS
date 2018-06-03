
// LFICloudDlg.cpp: Implementierung der Klasse LFICloudDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFICloudDlg
//

LFICloudDlg::LFICloudDlg(const ICloud& iCloud, CWnd* pParentWnd)
	: LFDialog(IDD_ICLOUD, pParentWnd)
{
	m_iCloudPaths = iCloud.m_iCloudPaths;
	m_FolderPath[0] = m_StoreName[0] = L'\0';
}

void LFICloudDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ICLOUD_DRIVE, m_wndDrivePanel);
	DDX_Control(pDX, IDC_ICLOUD_PHOTOS, m_wndPhotosPanel);
}

BOOL LFICloudDlg::InitDialog()
{
	// Item panels
	m_wndDrivePanel.SetItem(m_iCloudPaths.Drive);
	m_wndPhotosPanel.SetItem(m_iCloudPaths.PhotoLibrary);

	// Disable buttons
	GetDlgItem(IDC_ICLOUD_ADDDRIVE)->EnableWindow(m_iCloudPaths.Drive[0]!=L'\0');
	GetDlgItem(IDC_ICLOUD_ADDPHOTOS)->EnableWindow(m_iCloudPaths.PhotoLibrary[0]!=L'\0');

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFICloudDlg, LFDialog)
	ON_BN_CLICKED(IDC_ICLOUD_ADDDRIVE, OnAddDrive)
	ON_BN_CLICKED(IDC_ICLOUD_ADDPHOTOS, OnAddPhotos)
END_MESSAGE_MAP()

void LFICloudDlg::OnAddDrive()
{
	ASSERT(m_iCloudPaths.Drive[0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_iCloudPaths.Drive);
	m_wndCategory[0].GetWindowText(m_StoreName, 256);

	OnOK();
}

void LFICloudDlg::OnAddPhotos()
{
	ASSERT(m_iCloudPaths.PhotoLibrary[0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_iCloudPaths.PhotoLibrary);
	m_wndCategory[1].GetWindowText(m_StoreName, 256);

	OnOK();
}
