
// LFBoxDlg.cpp: Implementierung der Klasse LFBoxDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFBoxDlg
//

LFBoxDlg::LFBoxDlg(const Box& Box, CWnd* pParentWnd)
	: LFDialog(IDD_BOX, pParentWnd)
{
	wcscpy_s(m_FolderPath, MAX_PATH, Box.m_Path);
}

void LFBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ITEMPANEL, m_wndPanel);
}

BOOL LFBoxDlg::InitDialog()
{
	// Item panel
	m_wndPanel.SetItem(m_FolderPath);

	// Disable button
	GetDlgItem(IDOK)->EnableWindow(m_FolderPath[0]!=L'\0');

	return TRUE;
}
