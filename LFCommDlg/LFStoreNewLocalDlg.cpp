
// LFStoreNewLocalDlg.cpp: Implementierung der Klasse LFStoreNewLocalDlg
//

#include "stdafx.h"
#include "LFStoreNewLocalDlg.h"
#include "LFStoreNewGeneralPage.h"
#include "LFStoreNewPathPage.h"


// LFStoreNewLocalDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

LFStoreNewLocalDlg::LFStoreNewLocalDlg(CWnd* pParentWnd, CHAR Volume)
	: CPropertySheet(_T(""), pParentWnd)
{
	m_ulSHChangeNotifyRegister = NULL;
	m_Volume = Volume;

	m_pPages[0] = new LFStoreNewGeneralPage(Volume);
	m_pPages[0]->Construct(IDD_STORENEW_GENERAL);

	m_pPages[1] = new LFStoreNewPathPage(Volume);
	m_pPages[1]->Construct(IDD_STORENEW_PATH);

	// Seiten hinzufügen
	for (UINT a=0; a<2; a++)
		AddPage(m_pPages[a]);

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}

BOOL LFStoreNewLocalDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam)==IDOK)
	{
		LFStoreDescriptor store;
		ZeroMemory(&store, sizeof(store));

		const LFStoreNewGeneralPage* pPage = (LFStoreNewGeneralPage*)m_pPages[0];
		pPage->m_wndStoreName.GetWindowText(store.StoreName, 256);
		pPage->m_wndStoreComment.GetWindowText(store.StoreComment, 256);

		store.StoreMode = m_IsRemovable ? pPage->m_wndMakeSearchable.GetCheck() ? LFStoreModeHybrid : LFStoreModeExternal : LFStoreModeInternal;
		store.AutoLocation = (m_Path[0]==L'\0');

		wcscpy_s(store.DatPath, MAX_PATH, m_Path);
		if (m_IsRemovable && store.AutoLocation)
			swprintf_s(store.DatPath, MAX_PATH, L"%c:\\", m_Volume);

		CWaitCursor csr;
		LFErrorBox(LFCreateStore(&store, pPage->m_wndMakeDefault.GetCheck()==TRUE, GetSafeHwnd()), GetSafeHwnd());
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}


BEGIN_MESSAGE_MAP(LFStoreNewLocalDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(WM_PATHCHANGED, OnPathChanged)
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_MESSAGE_MAP()

BOOL LFStoreNewLocalDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDI_STORENEW));
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titel
	CString title;
	if (m_Volume)
	{
		CString mask;
		ENSURE(mask.LoadString(IDS_STORENEW_VOLUME));

		title.Format(mask, m_Volume);
	}
	else
	{
		ENSURE(title.LoadString(IDS_STORENEW));
	}

	SetWindowText(title);

	// Benachrichtigung, wenn sich Laufwerke ändern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
		SHCNE_DRIVEREMOVED | SHCNE_MEDIAREMOVED | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);

	// Einstellungen
	OnPathChanged();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStoreNewLocalDlg::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	for (UINT a=0; a<2; a++)
	{
		m_pPages[a]->DestroyWindow();
		delete m_pPages[a];
	}
}

void LFStoreNewLocalDlg::OnPathChanged()
{
	LFStoreNewGeneralPage* pPage0 = (LFStoreNewGeneralPage*)m_pPages[0];
	LFStoreNewPathPage* pPage1 = (LFStoreNewPathPage*)m_pPages[1];

	CHAR Drive = m_Volume;
	m_IsRemovable = m_Volume;
	m_Path[0] = L'\0';

	if (IsWindow(pPage1->m_wndAutoPath))
		if (!pPage1->m_wndAutoPath.GetCheck())
		{
			pPage1->m_wndPathTree.GetSelectedPath(m_Path);
			if (m_Path[0])
			{
				Drive = m_Path[0] & 0xFF;

				if (m_Path[wcslen(m_Path)-1]!=L'\\')
					wcscat_s(m_Path, MAX_PATH, L"\\");

				// Removeable?
				m_IsRemovable = LFGetLogicalDrives(LFGLD_External) & (1<<(m_Path[0]-L'A'));
			}
		}

	pPage0->m_wndIcon.SetCoreIcon(m_IsRemovable ? LFGetSourceForDrive(Drive)+1 : IDI_STR_Internal);
	pPage0->m_wndMakeSearchable.EnableWindow(m_IsRemovable);
}

LRESULT LFStoreNewLocalDlg::OnShellChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_Volume)
	{
		// Wenn das Laufwerk nicht mehr vorhanden ist, Dialog schließen
		WCHAR szDriveRoot[] = L" :\\";
		szDriveRoot[0] = m_Volume;

		UINT uDriveType = GetDriveType(szDriveRoot);
		if ((uDriveType==DRIVE_UNKNOWN) || (uDriveType==DRIVE_NO_ROOT_DIR))
			EndDialog(IDCANCEL);
	}

	return NULL;
}
