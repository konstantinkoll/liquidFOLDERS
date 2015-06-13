
// LFStoreNewDlg.cpp: Implementierung der Klasse LFStoreNewDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFStoreNewDlg.h"
#include "LFStoreNewGeneralPage.h"
#include "LFStoreNewPathPage.h"


// LFStoreNewDlg
//

LFStoreNewDlg::LFStoreNewDlg(CWnd* pParentWnd)
	: CPropertySheet(IDS_STORENEW, pParentWnd)
{
	m_pPages[0] = new LFStoreNewGeneralPage();
	m_pPages[0]->Construct(IDD_STORENEW_GENERAL);

	m_pPages[1] = new LFStoreNewPathPage();
	m_pPages[1]->Construct(IDD_STORENEW_PATH);

	// Seiten hinzufügen
	for (UINT a=0; a<2; a++)
		AddPage(m_pPages[a]);

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}

BOOL LFStoreNewDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam)==IDOK)
	{
		LFStoreDescriptor store;
		ZeroMemory(&store, sizeof(store));

		const LFStoreNewGeneralPage* pPage = (LFStoreNewGeneralPage*)m_pPages[0];
		pPage->m_wndStoreName.GetWindowText(store.StoreName, 256);
		pPage->m_wndStoreComment.GetWindowText(store.StoreComment, 256);

		store.Mode = LFStoreModeBackendInternal | (m_IsRemovable ? pPage->m_wndMakeSearchable.GetCheck() ? LFStoreModeIndexHybrid : LFStoreModeIndexExternal : LFStoreModeIndexInternal);
		if (m_Path[0]==L'\0')
			store.Flags |= LFStoreFlagAutoLocation;

		wcscpy_s(store.DatPath, MAX_PATH, m_Path);
		if (m_IsRemovable && (store.Flags & LFStoreFlagAutoLocation))
			swprintf_s(store.DatPath, MAX_PATH, L"%c:\\", m_Path[0]);

		CWaitCursor csr;
		LFErrorBox(LFCreateStore(&store), GetSafeHwnd());
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}


BEGIN_MESSAGE_MAP(LFStoreNewDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(WM_PATHCHANGED, OnPathChanged)
END_MESSAGE_MAP()

BOOL LFStoreNewDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LFGetApp()->LoadDialogIcon(IDI_STORENEW);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Einstellungen
	OnPathChanged();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStoreNewDlg::OnDestroy()
{
	for (UINT a=0; a<2; a++)
	{
		m_pPages[a]->DestroyWindow();
		delete m_pPages[a];
	}
}

void LFStoreNewDlg::OnPathChanged()
{
	LFStoreNewGeneralPage* pPage0 = (LFStoreNewGeneralPage*)m_pPages[0];
	LFStoreNewPathPage* pPage1 = (LFStoreNewPathPage*)m_pPages[1];

	UINT Icon = IDI_STR_LIQUIDFOLDERS;
	m_IsRemovable = FALSE;
	m_Path[0] = L'\0';

	if (IsWindow(pPage1->m_wndAutoPath))
		if (!pPage1->m_wndAutoPath.GetCheck())
		{
			pPage1->m_wndPathTree.GetSelectedPath(m_Path);
			if (m_Path[0])
			{
				Icon = LFGetSourceForVolume(m_Path[0] & 0xFF);

				if (m_Path[wcslen(m_Path)-1]!=L'\\')
					wcscat_s(m_Path, MAX_PATH, L"\\");

				// Removeable?
				m_IsRemovable = LFGetLogicalVolumes(LFGLV_External) & (1<<(m_Path[0]-L'A'));
			}
		}

	pPage0->m_wndIcon.SetCoreIcon(Icon);
	pPage0->m_wndMakeSearchable.EnableWindow(m_IsRemovable);
}
