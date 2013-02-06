
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

LFStorePropertiesDlg::LFStorePropertiesDlg(CHAR* StoreID, CWnd* pParent)
	: CPropertySheet(_T(""), pParent)
{
	if (LFGetStoreSettings(StoreID, &m_Store)!=LFOk)
		ZeroMemory(&m_Store, sizeof(m_Store));

	m_PageCount = 3;

	// Immer #0
	m_Pages[0] = new LFStorePropertiesGeneralPage(&m_Store);
	m_Pages[0]->Construct(IDD_STOREPROPERTIES_GENERAL);

	m_Pages[1] = new LFStorePropertiesToolsPage(&m_Store);
	m_Pages[1]->Construct(IDD_STOREPROPERTIES_TOOLS);

	m_Pages[2] = new LFStorePropertiesIndexPage(&m_Store);
	m_Pages[2]->Construct(IDD_STOREPROPERTIES_INDEX);

	// Seiten hinzufügen
	for (UINT a=0; a<m_PageCount; a++)
		AddPage(m_Pages[a]);

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
}

BOOL LFStorePropertiesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam)==IDOK)
	{
		CString name;
		m_Pages[0]->GetDlgItem(IDC_STORENAME)->GetWindowText(name);
		CString comment;
		m_Pages[0]->GetDlgItem(IDC_STORECOMMENT)->GetWindowText(comment);

		UINT res = LFSetStoreAttributes(m_Store.StoreID, name.GetBuffer(), comment.GetBuffer());
		if (res!=LFOk)
		{
			LFErrorBox(res, GetSafeHwnd());
			return TRUE;
		}
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, CPropertySheet)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL LFStorePropertiesDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDI_STOREPROPERTIES));
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titel
	CString mask;
	ENSURE(mask.LoadString(IDS_STOREPROPERTIES));

	CString title;
	title.Format(mask, m_Store.StoreName);
	SetWindowText(title);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStorePropertiesDlg::OnDestroy()
{
	for (UINT a=0; a<m_PageCount; a++)
	{
		m_Pages[a]->DestroyWindow();
		delete m_Pages[a];
	}
}
