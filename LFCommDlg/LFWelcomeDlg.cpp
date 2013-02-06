
// LFWelcomeDlg.cpp: Implementierung der Klasse LFWelcomeDlg
//

#include "stdafx.h"
#include "LFWelcomeDlg.h"
#include "LFCore.h"
#include "resource.h"


// LFWelcomeDlg
//

LFWelcomeDlg::LFWelcomeDlg(CWnd* pParentWnd)
	: LFDialog(IDD_WELCOME, LFDS_Default, pParentWnd)
{
}


BEGIN_MESSAGE_MAP(LFWelcomeDlg, LFDialog)
END_MESSAGE_MAP()

BOOL LFWelcomeDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	CheckLicenseKey();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFWelcomeDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		CWaitCursor csr;
		CreateStore(IDC_STORENAME1);
		CreateStore(IDC_STORENAME2);
		CreateStore(IDC_STORENAME3);
	}
}

void LFWelcomeDlg::CreateStore(INT ID)
{
	LFStoreDescriptor store;
	ZeroMemory(&store, sizeof(store));

	store.AutoLocation = TRUE;
	store.StoreMode = LFStoreModeInternal;
	GetDlgItem(ID)->GetWindowText(store.StoreName, 256);

	if (store.StoreName[0]!=L'\0')
		LFCreateStore(&store);
}
