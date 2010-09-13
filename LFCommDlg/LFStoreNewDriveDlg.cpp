
// LFStoreNewDriveDlg.cpp: Implementierung der Klasse LFStoreNewDriveDlg
//

#include "StdAfx.h"
#include "LFStoreNewDriveDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"
#include "..\\LFCore\\resource.h"


// LFStoreNewDriveDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define WM_USER_MEDIACHANGED       WM_USER+2

LFStoreNewDriveDlg::LFStoreNewDriveDlg(CWnd* pParentWnd, char Drive, LFStoreDescriptor* _store)
	: CDialog(IDD_STORENEWDRIVE, pParentWnd)
{
	store = _store;
	m_ulSHChangeNotifyRegister = NULL;
	m_Drive = Drive;
}


BEGIN_MESSAGE_MAP(LFStoreNewDriveDlg, CDialog)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_USER_MEDIACHANGED, OnMediaChanged)
END_MESSAGE_MAP()

BOOL LFStoreNewDriveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STORENEW));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Status und Laufwerke
	((CButton*)GetDlgItem(IDC_EXTERNALSTORE))->SetCheck(TRUE);

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, m_Drive);
	SetWindowText(caption);

	// Icons
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

void LFStoreNewDriveDlg::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	CDialog::OnDestroy();
}

LRESULT LFStoreNewDriveDlg::OnMediaChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Wenn das Laufwerk nicht mehr vorhanden ist, Dialog schließen
	wchar_t szDriveRoot[] = L" :\\";
	szDriveRoot[0] = m_Drive;

	UINT uDriveType = GetDriveType(szDriveRoot);
	if ((uDriveType==DRIVE_UNKNOWN) || (uDriveType==DRIVE_NO_ROOT_DIR))
			EndDialog(IDCANCEL);

	return NULL;
}

void LFStoreNewDriveDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_HYBRIDSTOREICON, m_IconHybrid);
	DDX_Control(pDX, IDC_EXTERNALSTOREICON, m_IconExternal);

	// Nur beim Verlassen des Dialogs
	if ((store) && (pDX->m_bSaveAndValidate))
	{
		// Pfad zusammenbauen
		CStringA Pfad;
	/*	if (m_nIDTemplate==IDD_STORENEW)
		{
			CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_DRIVELIST);
			int FocusItem = li->GetNextItem(-1, LVNI_FOCUSED);
			Pfad = ((char)li->GetItemData(FocusItem));
		}
		else
		{
			Pfad = m_Drive;
		}*/
		Pfad += _T(":\\");

		// LFStoreDescriptor ausfüllen
		GetDlgItem(IDC_STORENAME)->GetWindowText(store->StoreName, 256);
		GetDlgItem(IDC_COMMENT)->GetWindowText(store->Comment, 256);
		DDX_Radio(pDX, IDC_INTERNALSTORE, store->StoreMode);
		store->AutoLocation = FALSE;
		strcpy_s(store->DatPath, MAX_PATH, Pfad);
	}
}
