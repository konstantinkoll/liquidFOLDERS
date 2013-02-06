
// LFStorePropertiesIndexPage.cpp: Implementierung der Klasse LFStorePropertiesIndexPage
//

#include "stdafx.h"
#include "LFStorePropertiesIndexPage.h"
#include "Resource.h"


// LFStorePropertiesIndexPage
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFStorePropertiesIndexPage::LFStorePropertiesIndexPage(LFStoreDescriptor* pStore)
	: CPropertyPage(IDD_STOREPROPERTIES_INDEX)
{
	ASSERT(pStore);

	p_Store = pStore;
}

void LFStorePropertiesIndexPage::DoDataExchange(CDataExchange* pDX)
{
}


BEGIN_MESSAGE_MAP(LFStorePropertiesIndexPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL LFStorePropertiesIndexPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
