
// LFStoreNewPathPage.cpp: Implementierung der Klasse LFStoreNewPathPage
//

#include "stdafx.h"
#include "LFStoreNewPathPage.h"
#include "LFStoreNewDlg.h"
#include "Resource.h"


// LFStoreNewPathPage
//

extern LFMessageIDs* MessageIDs;

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
	m_wndPathTree.SetRootPath(CETR_AllVolumes);

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
