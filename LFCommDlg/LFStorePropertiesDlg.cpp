
// LFStorePropertiesDlg.cpp: Implementierung der Klasse LFStorePropertiesDlg
//

#include "stdafx.h"
#include "LFStorePropertiesDlg.h"
#include "LFStorePropertiesGeneralPage.h"
#include "LFStorePropertiesToolsPage.h"
#include "LFStorePropertiesIndexPage.h"


// LFStorePropertiesDlg
//

LFStorePropertiesDlg::LFStorePropertiesDlg(CHAR* StoreID, CWnd* pParentWnd)
	: CPropertySheet(_T(""), pParentWnd)
{
	if (LFGetStoreSettings(StoreID, &m_Store)==LFOk)
	{
		m_StoreID = m_Store.guid;
		m_StoreValid = TRUE;
	}
	else
	{
		ZeroMemory(&m_StoreID, sizeof(m_StoreID));
		m_StoreValid = FALSE;
	}

	m_PageCount = 3;

	// Immer #0
	m_pPages[0] = new LFStorePropertiesGeneralPage(&m_Store, &m_StoreValid);
	m_pPages[0]->Construct(IDD_STOREPROPERTIES_GENERAL);

	m_pPages[1] = new LFStorePropertiesToolsPage(&m_Store, &m_StoreValid);
	m_pPages[1]->Construct(IDD_STOREPROPERTIES_TOOLS);

	m_pPages[2] = new LFStorePropertiesIndexPage(&m_Store, &m_StoreValid);
	m_pPages[2]->Construct(IDD_STOREPROPERTIES_INDEX);

	// Seiten hinzufügen
	for (UINT a=0; a<m_PageCount; a++)
		AddPage(m_pPages[a]);

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}

BOOL LFStorePropertiesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam)==IDOK)
	{
		CWaitCursor csr;

		const LFStorePropertiesGeneralPage* pPage = (LFStorePropertiesGeneralPage*)m_pPages[0];

		CString Name;
		pPage->m_wndStoreName.GetWindowText(Name);
		CString Comment;
		pPage->m_wndStoreComment.GetWindowText(Comment);

		UINT Result = LFSetStoreAttributes(m_Store.StoreID, Name.GetBuffer(), Comment.GetBuffer());
		if (Result!=LFOk)
		{
			LFErrorBox(Result, GetSafeHwnd());
			return TRUE;
		}

		if (pPage->m_wndMakeDefault.IsWindowEnabled() && pPage->m_wndMakeDefault.GetCheck())
			LFErrorBox(LFMakeDefaultStore(m_Store.StoreID), GetSafeHwnd());

		ASSERT(pPage->m_wndMakeSearchable.IsWindowVisible()==((m_Store.Mode & LFStoreModeIndexMask)>=LFStoreModeIndexHybrid));

		if (pPage->m_wndMakeSearchable.IsWindowVisible())
			LFErrorBox(LFMakeStoreSearchable(m_Store.StoreID, pPage->m_wndMakeSearchable.GetCheck()==TRUE), GetSafeHwnd());
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}

void LFStorePropertiesDlg::UpdateStore(UINT Message, WPARAM wParam, LPARAM lParam)
{
	m_StoreValid = (LFGetStoreSettings(m_StoreID, &m_Store)==LFOk);
	GetDlgItem(IDOK)->EnableWindow(m_StoreValid);

	for (UINT a=0; a<m_PageCount; a++)
		if (IsWindow(m_pPages[a]->GetSafeHwnd()))
			m_pPages[a]->SendMessage(Message, wParam, lParam);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StatisticsChanged, OnStatisticsChanged)
END_MESSAGE_MAP()

BOOL LFStorePropertiesDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LFGetApp()->LoadDialogIcon(IDI_STOREPROPERTIES);
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Titel
	CString title;
	title.Format(IDS_STOREPROPERTIES, m_Store.StoreName);
	SetWindowText(title);

	// Button
	GetDlgItem(IDOK)->EnableWindow(m_StoreValid);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStorePropertiesDlg::OnDestroy()
{
	for (UINT a=0; a<m_PageCount; a++)
	{
		m_pPages[a]->DestroyWindow();
		delete m_pPages[a];
	}
}

LRESULT LFStorePropertiesDlg::OnStoresChanged(WPARAM wParam, LPARAM lParam)
{
	UpdateStore(LFGetApp()->p_MessageIDs->StoresChanged, wParam, lParam);

	return NULL;
}

LRESULT LFStorePropertiesDlg::OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam)
{
	UpdateStore(LFGetApp()->p_MessageIDs->StoreAttributesChanged, wParam, lParam);

	return NULL;
}

LRESULT LFStorePropertiesDlg::OnStatisticsChanged(WPARAM wParam, LPARAM lParam)
{
	UpdateStore(LFGetApp()->p_MessageIDs->StatisticsChanged, wParam, lParam);

	return NULL;
}
