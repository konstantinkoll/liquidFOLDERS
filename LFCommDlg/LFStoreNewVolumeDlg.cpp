
// LFStoreNewVolumeDlg.cpp: Implementierung der Klasse LFStoreNewVolumeDlg
//

#include "StdAfx.h"
#include "LFStoreNewVolumeDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"
#include "CExplorerTree.h"
#include "..\\LFCore\\resource.h"


// LFStoreNewVolumeDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFStoreNewVolumeDlg::LFStoreNewVolumeDlg(CWnd* pParentWnd, CHAR Drive, LFStoreDescriptor* pStore)
	: CDialog(IDD_STORENEWVOLUME, pParentWnd)
{
	m_pStore = pStore;
	m_ulSHChangeNotifyRegister = NULL;
	m_Drive = Drive;
}

void LFStoreNewVolumeDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_HYBRIDSTOREICON, m_IconHybrid);
	DDX_Control(pDX, IDC_EXTERNALSTOREICON, m_IconExternal);
	DDX_Control(pDX, IDC_PATHTREE, m_PathTree);

	// Nur beim Verlassen des Dialogs
	if (pDX->m_bSaveAndValidate)
	{
		// Pfad zusammenbauen
		WCHAR Path[MAX_PATH];
		m_PathTree.GetSelectedPath(Path);
		if (Path[0])
			if (Path[wcslen(Path)-1]!=L'\\')
				wcscat_s(Path, MAX_PATH, L"\\");

		// LFStoreDescriptor ausf¸llen
		GetDlgItem(IDC_STORENAME)->GetWindowText(m_pStore->StoreName, 256);
		GetDlgItem(IDC_COMMENT)->GetWindowText(m_pStore->Comment, 256);

		m_pStore->StoreMode = ((CButton*)GetDlgItem(IDC_HYBRIDSTORE))->GetCheck() ? LFStoreModeHybrid : LFStoreModeExternal;
		m_pStore->AutoLocation = FALSE;
		wcscpy_s(m_pStore->DatPath, MAX_PATH, Path);
	}
}


BEGIN_MESSAGE_MAP(LFStoreNewVolumeDlg, CDialog)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_MESSAGE_MAP()

BOOL LFStoreNewVolumeDlg::OnInitDialog()
{
	((LFApplication*)AfxGetApp())->ShowNagScreen();

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
		SHCNE_DRIVEREMOVED | SHCNE_MEDIAREMOVED | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);

	return TRUE;
}

void LFStoreNewVolumeDlg::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	CDialog::OnDestroy();
}

LRESULT LFStoreNewVolumeDlg::OnShellChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Wenn das Laufwerk nicht mehr vorhanden ist, Dialog schlieﬂen
	WCHAR szDriveRoot[] = L" :\\";
	szDriveRoot[0] = m_Drive;

	UINT uDriveType = GetDriveType(szDriveRoot);
	if ((uDriveType==DRIVE_UNKNOWN) || (uDriveType==DRIVE_NO_ROOT_DIR))
		EndDialog(IDCANCEL);

	return NULL;
}
