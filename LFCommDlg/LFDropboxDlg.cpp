
// LFDropboxDlg.cpp: Implementierung der Klasse LFDropboxDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFDropboxDlg
//

LFDropboxDlg::LFDropboxDlg(const Dropbox& Dropbox, CWnd* pParentWnd)
	: LFDialog(IDD_DROPBOX, pParentWnd)
{
	m_DropboxData = Dropbox.m_DropboxData;
	m_FolderPath[0] = L'\0';
}

void LFDropboxDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DROPBOX_PERSONAL, m_wndPersonalPanel);
	DDX_Control(pDX, IDC_DROPBOX_BUSINESS, m_wndBusinessPanel);
}

BOOL LFDropboxDlg::InitDialog()
{
	// Item panels
	m_wndPersonalPanel.SetItem(m_DropboxData.Paths[0], IDS_SUBSCRIPTION, m_DropboxData.SubscriptionTypes[0]);
	m_wndBusinessPanel.SetItem(m_DropboxData.Paths[1], IDS_SUBSCRIPTION, m_DropboxData.SubscriptionTypes[1]);

	// Disable buttons
	GetDlgItem(IDC_DROPBOX_ADDPERSONAL)->EnableWindow(m_DropboxData.Paths[0][0]!=L'\0');
	GetDlgItem(IDC_DROPBOX_ADDBUSINESS)->EnableWindow(m_DropboxData.Paths[1][0]!=L'\0');

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFDropboxDlg, LFDialog)
	ON_BN_CLICKED(IDC_DROPBOX_ADDPERSONAL, OnAddPersonal)
	ON_BN_CLICKED(IDC_DROPBOX_ADDBUSINESS, OnAddBusiness)
END_MESSAGE_MAP()

void LFDropboxDlg::OnAddPersonal()
{
	ASSERT(m_DropboxData.Paths[0][0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_DropboxData.Paths[0]);

	OnOK();
}

void LFDropboxDlg::OnAddBusiness()
{
	ASSERT(m_DropboxData.Paths[1][0]!=L'\0');

	wcscpy_s(m_FolderPath, MAX_PATH, m_DropboxData.Paths[1]);

	OnOK();
}
