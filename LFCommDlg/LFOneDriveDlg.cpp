
// LFOneDriveDlg.cpp: Implementierung der Klasse LFOneDriveDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFOneDriveDlg
//

LFOneDriveDlg::LFOneDriveDlg(const OneDrive& OneDrive, CWnd* pParentWnd)
	: LFDialog(IDD_ONEDRIVE, pParentWnd)
{
	m_OneDrivePaths = OneDrive.m_OneDrivePaths;
	m_FolderPath[0] = L'\0';
}

void LFOneDriveDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ONEDRIVE_ONEDRIVE, m_wndOneDrivePanel);
	DDX_Control(pDX, IDC_ONEDRIVE_CAMERAROLL, m_wndCameraRollPanel);
	DDX_Control(pDX, IDC_ONEDRIVE_DOCUMENTS, m_wndDocumentsPanel);
	DDX_Control(pDX, IDC_ONEDRIVE_PICTURES, m_wndPicturesPanel);
}

BOOL LFOneDriveDlg::InitDialog()
{
	// Item panels
	m_wndOneDrivePanel.SetItem(m_OneDrivePaths.OneDrive);
	m_wndCameraRollPanel.SetItem(m_OneDrivePaths.CameraRoll);
	m_wndDocumentsPanel.SetItem(m_OneDrivePaths.Documents);
	m_wndPicturesPanel.SetItem(m_OneDrivePaths.Pictures);

	// Disable buttons
	GetDlgItem(IDC_ONEDRIVE_ADDONEDRIVE)->EnableWindow(m_OneDrivePaths.OneDrive[0]!=L'\0');
	GetDlgItem(IDC_ONEDRIVE_ADDCAMERAROLL)->EnableWindow(m_OneDrivePaths.CameraRoll[0]!=L'\0');
	GetDlgItem(IDC_ONEDRIVE_ADDDOCUMENTS)->EnableWindow(m_OneDrivePaths.Documents[0]!=L'\0');
	GetDlgItem(IDC_ONEDRIVE_ADDPICTURES)->EnableWindow(m_OneDrivePaths.Pictures[0]!=L'\0');

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFOneDriveDlg, LFDialog)
	ON_BN_CLICKED(IDC_ONEDRIVE_ADDONEDRIVE, OnAddOneDrive)
	ON_BN_CLICKED(IDC_ONEDRIVE_ADDCAMERAROLL, OnAddCameraRoll)
	ON_BN_CLICKED(IDC_ONEDRIVE_ADDDOCUMENTS, OnAddDocuments)
	ON_BN_CLICKED(IDC_ONEDRIVE_ADDPICTURES, OnAddPictures)
END_MESSAGE_MAP()

void LFOneDriveDlg::OnAddOneDrive()
{
	ASSERT(m_OneDrivePaths.OneDrive[0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_OneDrivePaths.OneDrive);

	OnOK();
}

void LFOneDriveDlg::OnAddCameraRoll()
{
	ASSERT(m_OneDrivePaths.CameraRoll[0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_OneDrivePaths.CameraRoll);

	OnOK();
}

void LFOneDriveDlg::OnAddDocuments()
{
	ASSERT(m_OneDrivePaths.Documents[0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_OneDrivePaths.Documents);

	OnOK();
}

void LFOneDriveDlg::OnAddPictures()
{
	ASSERT(m_OneDrivePaths.Pictures[0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_OneDrivePaths.Pictures);

	OnOK();
}
