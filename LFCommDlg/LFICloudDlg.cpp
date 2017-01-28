
// LFICloudDlg.cpp: Implementierung der Klasse LFICloudDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFICloudDlg
//

LFICloudDlg::LFICloudDlg(const ICloud& ICloud, CWnd* pParentWnd)
	: LFDialog(IDD_ICLOUD, pParentWnd)
{
	wcscpy_s(m_FolderPath, MAX_PATH, ICloud.m_Path);
}

void LFICloudDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ITEMPANEL, m_wndPanel);
}

BOOL LFICloudDlg::InitDialog()
{
	// Item panel
	m_wndPanel.SetItem(m_FolderPath);

	// Disable button
	GetDlgItem(IDOK)->EnableWindow(m_FolderPath[0]!=L'\0');

	return TRUE;
}
