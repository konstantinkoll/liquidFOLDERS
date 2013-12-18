
// LFStoreNewPathPage.cpp: Implementierung der Klasse LFStoreNewPathPage
//

#include "stdafx.h"
#include "LFStoreNewPathPage.h"
#include "LFStoreNewLocalDlg.h"
#include "Resource.h"


// LFStoreNewPathPage
//

extern LFMessageIDs* MessageIDs;

LFStoreNewPathPage::LFStoreNewPathPage(CHAR Volume)
	: CPropertyPage()
{
	m_Volume = Volume;
}

void LFStoreNewPathPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_AUTOPATH, m_wndAutoPath);
	DDX_Control(pDX, IDC_PATHTREE, m_wndPathTree);
}


BEGIN_MESSAGE_MAP(LFStoreNewPathPage, CPropertyPage)
	ON_BN_CLICKED(IDC_AUTOPATH, OnAutoPath)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PATHTREE, OnSelChanged)
END_MESSAGE_MAP()

BOOL LFStoreNewPathPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Checkbox
	m_wndAutoPath.SetCheck(TRUE);

	// Tree
	m_wndPathTree.SetOnlyFilesystem(TRUE);
	if (m_Volume)
	{
		CString tmpStr;
		tmpStr.Format(_T("%C:\\"), m_Volume);
		m_wndPathTree.SetRootPath(tmpStr);
	}
	else
	{
		m_wndPathTree.SetRootPath(CETR_AllDrives);
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStoreNewPathPage::OnAutoPath()
{
	m_wndPathTree.EnableWindow(!m_wndAutoPath.GetCheck());
	GetOwner()->SendMessage(WM_PATHCHANGED);
}

void LFStoreNewPathPage::OnSelChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	GetOwner()->SendMessage(WM_PATHCHANGED);
}
