
// LFStoreNewDlg.cpp: Implementierung der Klasse LFStoreNewDlg
//

#include "StdAfx.h"
#include "LFStoreNewDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"
#include "..\\LFCore\\resource.h"


// LFStoreNewDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define WM_USER_MEDIACHANGED       WM_USER+2

LFStoreNewDlg::LFStoreNewDlg(CWnd* pParentWnd, LFStoreDescriptor* pStore)
	: CDialog(IDD_STORENEW, pParentWnd)
{
	ASSERT(pStore);

	m_pStore = pStore;
	m_ulSHChangeNotifyRegister = NULL;
}

void LFStoreNewDlg::SetOkButton()
{
	char Path[MAX_PATH];
	GetDlgItem(IDOK)->EnableWindow((!m_PathTree.IsWindowEnabled()) || (m_PathTree.GetSelectedPathA(Path)));
}

void LFStoreNewDlg::PopulateTreeCtrl()
{
	m_PathTree.SetRootPath(((CButton*)GetDlgItem(IDC_INTERNALSTORE))->GetCheck() ? CETR_InternalDrives : CETR_ExternalDrives);
	SetOkButton();
}

void LFStoreNewDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_INTERNALSTOREICON, m_IconInternal);
	DDX_Control(pDX, IDC_HYBRIDSTOREICON, m_IconHybrid);
	DDX_Control(pDX, IDC_EXTERNALSTOREICON, m_IconExternal);
	DDX_Control(pDX, IDC_PATHTREE, m_PathTree);

	// Nur beim Verlassen des Dialogs
	if (pDX->m_bSaveAndValidate)
	{
		// Pfad zusammenbauen
		char Path[MAX_PATH];
		m_PathTree.GetSelectedPathA(Path);
		if (Path[0])
			if (Path[strlen(Path)-1]!='\\')
				strcat_s(Path, MAX_PATH, "\\");

		// LFStoreDescriptor ausfüllen
		GetDlgItem(IDC_STORENAME)->GetWindowText(m_pStore->StoreName, 256);
		GetDlgItem(IDC_COMMENT)->GetWindowText(m_pStore->Comment, 256);
		DDX_Radio(pDX, IDC_INTERNALSTORE, m_pStore->StoreMode);

		if (m_pStore->StoreMode==LFStoreModeInternal)
		{
			MakeDefault = ((CButton*)GetDlgItem(IDC_MAKEDEFAULT))->GetCheck()!=0;
			m_pStore->AutoLocation = ((CButton*)GetDlgItem(IDC_AUTOPATH))->GetCheck()!=0;
			strcpy_s(m_pStore->DatPath, MAX_PATH, m_pStore->AutoLocation ? "" : Path);
		}
		else
		{
			MakeDefault = FALSE;
			m_pStore->AutoLocation = FALSE;
			strcpy_s(m_pStore->DatPath, MAX_PATH, Path);
		}
	}
}


BEGIN_MESSAGE_MAP(LFStoreNewDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MAKEDEFAULT, OnSetInternalIcon)
	ON_BN_CLICKED(IDC_INTERNALSTORE, OnSetOptions)
	ON_BN_CLICKED(IDC_HYBRIDSTORE, OnSetOptions)
	ON_BN_CLICKED(IDC_EXTERNALSTORE, OnSetOptions)
	ON_BN_CLICKED(IDC_AUTOPATH, OnSetOptions)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PATHTREE, OnSelChanged)
	ON_MESSAGE(WM_USER_MEDIACHANGED, OnMediaChanged)
END_MESSAGE_MAP()

BOOL LFStoreNewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STORENEW));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Status
	((CButton*)GetDlgItem(IDC_INTERNALSTORE))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_AUTOPATH))->SetCheck(TRUE);

	// Pfad
	m_PathTree.SetOnlyFilesystem(TRUE);
	PopulateTreeCtrl();

	// Icons
	OnSetInternalIcon();
	m_IconHybrid.SetCoreIcon(IDI_STORE_Bag);
	m_IconExternal.SetCoreIcon(IDI_STORE_Bag);

	// Benachrichtigung, wenn sich Laufwerke ändern
	LPITEMIDLIST pidl;
	if (SUCCEEDED(SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &pidl)))
	{
		SHChangeNotifyEntry shCNE;
		shCNE.pidl = pidl;
		shCNE.fRecursive = TRUE;

		m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_ShellLevel,
			SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
			WM_USER_MEDIACHANGED, 1, &shCNE);
	}

	return TRUE;
}

void LFStoreNewDlg::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	CDialog::OnDestroy();
}

void LFStoreNewDlg::OnSetInternalIcon()
{
	m_IconInternal.SetCoreIcon(((CButton*)GetDlgItem(IDC_MAKEDEFAULT))->GetCheck() ? IDI_STORE_Default : IDI_STORE_Internal);
}

void LFStoreNewDlg::OnSetOptions()
{
	BOOL b = ((CButton*)GetDlgItem(IDC_INTERNALSTORE))->GetCheck();
	GetDlgItem(IDC_MAKEDEFAULT)->EnableWindow(b);
	GetDlgItem(IDC_AUTOPATH)->EnableWindow(b);

	if (b)
		b ^= !((CButton*)GetDlgItem(IDC_AUTOPATH))->GetCheck();
	GetDlgItem(IDC_PATHTREE)->EnableWindow(!b);

	PopulateTreeCtrl();
}

void LFStoreNewDlg::OnSelChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	SetOkButton();
}

LRESULT LFStoreNewDlg::OnMediaChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	PopulateTreeCtrl();

	return NULL;
}
