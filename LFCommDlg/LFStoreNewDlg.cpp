
// LFStoreNewDlg.cpp: Implementierung der Klasse LFStoreNewDlg
//

#include "StdAfx.h"
#include "LFStoreNewDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"


// LFStoreNewDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define WM_USER_MEDIACHANGED       WM_USER+2

LFStoreNewDlg::LFStoreNewDlg(CWnd* pParentWnd, UINT nIDTemplate, char Drive, LFStoreDescriptor* _store)
	: CDialog(nIDTemplate, pParentWnd)
{
	store = _store;
	m_ulSHChangeNotifyRegister = NULL;
	m_nIDTemplate = nIDTemplate;
	m_Drive = Drive;
}


BEGIN_MESSAGE_MAP(LFStoreNewDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MAKEDEFAULT, SetInternalIcon)
	ON_BN_CLICKED(IDC_INTERNALSTORE, SetOptions)
	ON_BN_CLICKED(IDC_HYBRIDSTORE, SetOptions)
	ON_BN_CLICKED(IDC_EXTERNALSTORE, SetOptions)
	ON_BN_CLICKED(IDC_AUTODRIVE, SetOptions)
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

	// Load icons
	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons!=NULL)
	{
		((LFApplication*)AfxGetApp())->ExtractCoreIcons(hModIcons, 16, &icons);
		FreeLibrary(hModIcons);
	}

	// Status und Laufwerke
	if (m_nIDTemplate==IDD_STORENEW)
	{
		((CButton*)GetDlgItem(IDC_INTERNALSTORE))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_AUTODRIVE))->SetCheck(TRUE);
		PopulateListCtrl();
	}
	else
	{
		((CButton*)GetDlgItem(IDC_EXTERNALSTORE))->SetCheck(TRUE);

		// Titelleiste
		CString text;
		GetWindowText(text);
		CString caption;
		caption.Format(text, m_Drive);
		SetWindowText(caption);
	}

	// Benachrichtigung, wenn sich Laufwerke ändern
	HWND hWnd = GetSafeHwnd();
	LPITEMIDLIST ppidl;
	if (SHGetSpecialFolderLocation(hWnd, CSIDL_DESKTOP, &ppidl)==NOERROR)
	{
		SHChangeNotifyEntry shCNE;
		shCNE.pidl = ppidl;
		shCNE.fRecursive = TRUE;

		m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWnd, SHCNRF_ShellLevel,
			SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
			WM_USER_MEDIACHANGED, 1, &shCNE);
		ASSERT(m_ulSHChangeNotifyRegister);
	}
	else
	{
		ASSERT(FALSE);
	}

	return TRUE;
}

void LFStoreNewDlg::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	CDialog::OnDestroy();
}

void LFStoreNewDlg::SetInternalIcon()
{
	if (m_nIDTemplate==IDD_STORENEW)
	{
		BOOL b = ((CButton*)GetDlgItem(IDC_MAKEDEFAULT))->GetCheck();
		GetDlgItem(b ? IDC_DEFAULTSTOREICON : IDC_INTERNALSTOREICON)->ShowWindow(SW_SHOW);
		GetDlgItem(b ? IDC_INTERNALSTOREICON : IDC_DEFAULTSTOREICON)->ShowWindow(SW_HIDE);
	}
}

void LFStoreNewDlg::SetOptions()
{
	if (m_nIDTemplate==IDD_STORENEW)
	{
		BOOL b = ((CButton*)GetDlgItem(IDC_INTERNALSTORE))->GetCheck();

		GetDlgItem(IDC_MAKEDEFAULT)->EnableWindow(b);
		GetDlgItem(IDC_AUTODRIVE)->EnableWindow(b);

		if (b)
			b ^= !((CButton*)GetDlgItem(IDC_AUTODRIVE))->GetCheck();

		GetDlgItem(IDC_DRIVELIST)->EnableWindow(!b);
		PopulateListCtrl();
	}
}

LRESULT LFStoreNewDlg::OnMediaChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_nIDTemplate==IDD_STORENEW)
	{
		PopulateListCtrl();
	}
	else
	{
		// Wenn das Laufwerk nicht mehr vorhanden ist, Dialog schließen
		wchar_t szDriveRoot[] = L" :\\";
		szDriveRoot[0] = m_Drive;
		UINT uDriveType = GetDriveType(szDriveRoot);
		if ((uDriveType==DRIVE_UNKNOWN) || (uDriveType==DRIVE_NO_ROOT_DIR))
			EndDialog(IDCANCEL);
	}
	return NULL;
}

void LFStoreNewDlg::DoDataExchange(CDataExchange* pDX)
{
	// DDX nur beim Verlassen des Dialogs
	if ((store) && (pDX->m_bSaveAndValidate))
	{
		// Pfad zusammenbauen
		CStringA Pfad;
		if (m_nIDTemplate==IDD_STORENEW)
		{
			CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_DRIVELIST);
			int FocusItem = li->GetNextItem(-1, LVNI_FOCUSED);
			Pfad = ((char)li->GetItemData(FocusItem));
		}
		else
		{
			Pfad = m_Drive;
		}
		Pfad += ":\\";

		// LFStoreDescriptor ausfüllen
		GetDlgItem(IDC_STORENAME)->GetWindowText(store->StoreName, 255);
		GetDlgItem(IDC_COMMENT)->GetWindowText(store->Comment, 255);
		DDX_Radio(pDX, IDC_INTERNALSTORE, store->StoreMode);
		if (store->StoreMode==LFStoreModeInternal)
		{
			makeDefault = ((CButton*)GetDlgItem(IDC_MAKEDEFAULT))->GetCheck()!=0;
			store->AutoLocation = ((CButton*)GetDlgItem(IDC_AUTODRIVE))->GetCheck()!=0;
			if (!store->AutoLocation)
				strcpy_s(store->DatPath, MAX_PATH, Pfad);
		}
		else
		{
			store->AutoLocation = FALSE;
			strcpy_s(store->DatPath, MAX_PATH, Pfad);
			makeDefault = FALSE;
		}
	}
}

void LFStoreNewDlg::PopulateListCtrl()
{
	BOOL fixed = ((CButton*)GetDlgItem(IDC_INTERNALSTORE))->GetCheck();

	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_DRIVELIST);

	li->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	int FocusItem = li->GetNextItem(-1, LVNI_FOCUSED);
	li->DeleteAllItems();
	li->SetImageList(&icons, LVSIL_SMALL);

	DWORD DrivesOnSystem = LFGetLogicalDrives(fixed ? LFGLD_Internal | LFGLD_Network : LFGLD_External);
	wchar_t szDriveRoot[] = L" :\\";
	int nIndex = 0;

	char SysDrive[MAX_PATH];
	GetWindowsDirectoryA(SysDrive, MAX_PATH);

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if (!(DrivesOnSystem & 1))
			continue;

		szDriveRoot[0] = cDrive;
		UINT uDriveType = GetDriveType(szDriveRoot);

		SHFILEINFO sfi;
			if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES))
			if (sfi.dwAttributes)
			{
				li->InsertItem(nIndex, sfi.szDisplayName, LFGetDriveIcon(cDrive, sfi.dwAttributes!=0)-1);
				li->SetItemData(nIndex, (DWORD)cDrive);
				nIndex++;
			}
	}

	if (nIndex)
	{
		if (FocusItem<0)
			FocusItem = 0;
		if (FocusItem>=nIndex)
			FocusItem = nIndex-1;
		li->SetItemState(FocusItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}

	GetDlgItem(IDOK)->EnableWindow((!li->IsWindowEnabled()) || (nIndex>0));
}
