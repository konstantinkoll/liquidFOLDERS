
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
		m_StoreID = m_Store.UniqueID;
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

		WCHAR StoreName[256];
		pPage->m_wndStoreName.GetWindowText(StoreName, 256);

		WCHAR Comment[256];
		pPage->m_wndStoreComment.GetWindowText(Comment, 256);

		UINT Result = LFSetStoreAttributes(m_Store.StoreID, StoreName, Comment);
		if (Result!=LFOk)
		{
			LFErrorBox(this, Result);
			return TRUE;
		}

		if (pPage->m_wndMakeDefault.IsWindowEnabled() && pPage->m_wndMakeDefault.GetCheck())
			LFErrorBox(this, LFSetDefaultStore(m_Store.StoreID));

		ASSERT(pPage->m_wndMakeSearchable.IsWindowVisible()==((m_Store.Mode & LFStoreModeIndexMask)>=LFStoreModeIndexHybrid));

		if (pPage->m_wndMakeSearchable.IsWindowVisible())
			LFErrorBox(this, LFMakeStoreSearchable(m_Store.StoreID, pPage->m_wndMakeSearchable.GetCheck()));
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}

void LFStorePropertiesDlg::UpdateStore(UINT Message, WPARAM wParam, LPARAM lParam)
{
	m_StoreValid = (LFGetStoreSettingsEx(m_StoreID, &m_Store)==LFOk);
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
