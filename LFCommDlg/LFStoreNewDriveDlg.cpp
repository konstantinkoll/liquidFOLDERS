
// LFStoreNewDriveDlg.cpp: Implementierung der Klasse LFStoreNewDriveDlg
//

#include "StdAfx.h"
#include "LFStoreNewDriveDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"
#include "CExplorerTree.h"
#include "..\\LFCore\\resource.h"


// LFStoreNewDriveDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define WM_USER_MEDIACHANGED       WM_USER+2

LFStoreNewDriveDlg::LFStoreNewDriveDlg(CWnd* pParentWnd, char Drive, LFStoreDescriptor* pStore)
	: CDialog(IDD_STORENEWDRIVE, pParentWnd)
{
	m_pStore = pStore;
	m_ulSHChangeNotifyRegister = NULL;
	m_Drive = Drive;
}

void LFStoreNewDriveDlg::DoDataExchange(CDataExchange* pDX)
{
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

		// LFStoreDescriptor ausf¸llen
		GetDlgItem(IDC_STORENAME)->GetWindowText(m_pStore->StoreName, 256);
		GetDlgItem(IDC_COMMENT)->GetWindowText(m_pStore->Comment, 256);

		m_pStore->StoreMode = ((CButton*)GetDlgItem(IDC_HYBRIDSTORE))->GetCheck() ? LFStoreModeHybrid : LFStoreModeExternal;
		m_pStore->AutoLocation = FALSE;
		strcpy_s(m_pStore->DatPath, MAX_PATH, Path);
	}
}

BEGIN_MESSAGE_MAP(LFStoreNewDriveDlg, CDialog)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_USER_MEDIACHANGED, OnMediaChanged)
END_MESSAGE_MAP()

BOOL LFStoreNewDriveDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol f¸r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STORENEW));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Titelleiste
	CString tmpStr;
	GetWindowText(tmpStr);
	CString caption;
	caption.Format(tmpStr, m_Drive);
	SetWindowText(caption);

	// Status
	((CButton*)GetDlgItem(IDC_EXTERNALSTORE))->SetCheck(TRUE);

	// Pfad
	m_PathTree.SetOnlyFilesystem(TRUE);
	tmpStr.Format(_T("%c:\\"), m_Drive);
	m_PathTree.SetRootPath(tmpStr);

	// Icons
	m_IconHybrid.SetCoreIcon(IDI_STORE_Bag);
	m_IconExternal.SetCoreIcon(IDI_STORE_Bag);

	// Benachrichtigung, wenn sich Laufwerke ‰ndern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);

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
	// Wenn das Laufwerk nicht mehr vorhanden ist, Dialog schlieﬂen
	wchar_t szDriveRoot[] = L" :\\";
	szDriveRoot[0] = m_Drive;

	UINT uDriveType = GetDriveType(szDriveRoot);
	if ((uDriveType==DRIVE_UNKNOWN) || (uDriveType==DRIVE_NO_ROOT_DIR))
		EndDialog(IDCANCEL);

	return NULL;
}
