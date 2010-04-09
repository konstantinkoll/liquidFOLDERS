#include "stdafx.h"
#include "LFWelcomeDlg.h"
#include "LFCore.h"
#include "resource.h"


LFWelcomeDlg::LFWelcomeDlg(CWnd* pParentWnd)
	: LFDialog(IDD_WELCOME, pParentWnd)
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
	DDX_Control(pDX, IDOK, m_OkButton);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_ENTERLICENSEKEY, m_LicenseButton);

	if (pDX->m_bSaveAndValidate)
	{
		CreateStore(IDC_STORENAME1);
		CreateStore(IDC_STORENAME2);
		CreateStore(IDC_STORENAME3);
	}
}

void LFWelcomeDlg::CreateStore(int ID)
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();
	s->AutoLocation = TRUE;
	s->StoreMode = LFStoreModeInternal;
	GetDlgItem(ID)->GetWindowText(s->StoreName, 255);

	if (wcscmp(s->StoreName, L"")!=0)
		LFCreateStore(s);

	LFFreeStoreDescriptor(s);
}
