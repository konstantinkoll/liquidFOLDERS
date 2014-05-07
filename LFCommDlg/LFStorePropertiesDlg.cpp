
// LFStorePropertiesDlg.cpp: Implementierung der Klasse LFStorePropertiesDlg
//

#include "stdafx.h"
#include "LFStorePropertiesDlg.h"
#include "LFStorePropertiesGeneralPage.h"
#include "LFStorePropertiesToolsPage.h"
#include "LFStorePropertiesIndexPage.h"


// LFStorePropertiesDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

LFStorePropertiesDlg::LFStorePropertiesDlg(CHAR* StoreID, CWnd* pParentWnd)
	: CPropertySheet(_T(""), pParentWnd)
{
	if (LFGetStoreSettings(StoreID, &m_Store)==LFOk)
	{
		m_Key = m_Store.guid;
		m_StoreValid = TRUE;
	}
	else
	{
		ZeroMemory(&m_Key, sizeof(m_Key));
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

	// Seiten hinzuf�gen
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

		UINT res = LFSetStoreAttributes(m_Store.StoreID, Name.GetBuffer(), Comment.GetBuffer(), GetSafeHwnd());
		if (res!=LFOk)
		{
			LFErrorBox(res, GetSafeHwnd());
			return TRUE;
		}

		if (pPage->m_wndMakeDefault.IsWindowEnabled() && pPage->m_wndMakeDefault.GetCheck())
			LFErrorBox(LFMakeDefaultStore(m_Store.StoreID, GetSafeHwnd()), GetSafeHwnd());

		ASSERT(pPage->m_wndMakeSearchable.IsWindowVisible()==(m_Store.IndexMode>=LFStoreModeIndexHybrid));

		if (pPage->m_wndMakeSearchable.IsWindowVisible())
			LFErrorBox(LFMakeStoreSearchable(m_Store.StoreID, pPage->m_wndMakeSearchable.GetCheck()==TRUE, GetSafeHwnd()), GetSafeHwnd());
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}

void LFStorePropertiesDlg::UpdateStore(UINT Message, WPARAM wParam, LPARAM lParam)
{
	if ((HWND)lParam!=GetSafeHwnd())
	{
		m_StoreValid = (LFGetStoreSettings(m_Key, &m_Store)==LFOk);
		GetDlgItem(IDOK)->EnableWindow(m_StoreValid);

		for (UINT a=0; a<m_PageCount; a++)
			if (IsWindow(m_pPages[a]->GetSafeHwnd()))
				m_pPages[a]->SendMessage(Message, wParam, lParam);
	}
}


BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(MessageIDs->StatisticsChanged, OnStatisticsChanged)
END_MESSAGE_MAP()

BOOL LFStorePropertiesDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDI_STOREPROPERTIES));
	SetIcon(hIcon, TRUE);		// Gro�es Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titel
	CString mask;
	ENSURE(mask.LoadString(IDS_STOREPROPERTIES));

	CString title;
	title.Format(mask, m_Store.StoreName);
	SetWindowText(title);

	// Button
	GetDlgItem(IDOK)->EnableWindow(m_StoreValid);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
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
	UpdateStore(MessageIDs->StoresChanged, wParam, lParam);

	return NULL;
}

LRESULT LFStorePropertiesDlg::OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam)
{
	UpdateStore(MessageIDs->StoreAttributesChanged, wParam, lParam);

	return NULL;
}

LRESULT LFStorePropertiesDlg::OnStatisticsChanged(WPARAM wParam, LPARAM lParam)
{
	UpdateStore(MessageIDs->StatisticsChanged, wParam, lParam);

	return NULL;
}
