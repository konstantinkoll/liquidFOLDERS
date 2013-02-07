
// LFStorePropertiesIndexPage.cpp: Implementierung der Klasse LFStorePropertiesIndexPage
//

#include "stdafx.h"
#include "LFStorePropertiesIndexPage.h"
#include "Resource.h"


// LFStorePropertiesIndexPage
//

extern LFMessageIDs* MessageIDs;

LFStorePropertiesIndexPage::LFStorePropertiesIndexPage(LFStoreDescriptor* pStore, BOOL* pStoreValid)
	: CPropertyPage(IDD_STOREPROPERTIES_INDEX)
{
	ASSERT(pStore);
	ASSERT(pValid);

	p_Store = pStore;
	p_StoreValid = pStoreValid;
}


BEGIN_MESSAGE_MAP(LFStorePropertiesIndexPage, CPropertyPage)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesIndexPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Store
	SendMessage(MessageIDs->StoresChanged);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

LRESULT LFStorePropertiesIndexPage::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_StoreValid)
	{
		WCHAR tmpStr[256];
		GetDlgItem(IDC_DATPATH)->SetWindowText(p_Store->DatPath);

		OLECHAR szGUID[MAX_PATH];
		StringFromGUID2(p_Store->guid, szGUID, MAX_PATH);
		GetDlgItem(IDC_GUID)->SetWindowText(szGUID);

		GetDlgItem(IDC_IDXPATHMAIN)->SetWindowText(p_Store->IdxPathMain);
		GetDlgItem(IDC_IDXPATHAUX)->SetWindowText(p_Store->IdxPathAux);

		LFUINTToString(p_Store->IndexVersion, tmpStr, 256);
		GetDlgItem(IDC_IDXVERSION)->SetWindowText(tmpStr);
	}

	return NULL;
}
