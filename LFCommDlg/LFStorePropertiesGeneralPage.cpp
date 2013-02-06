
// LFStorePropertiesGeneralPage.cpp: Implementierung der Klasse LFStorePropertiesGeneralPage
//

#include "stdafx.h"
#include "LFStorePropertiesGeneralPage.h"
#include "Resource.h"


// LFStorePropertiesGeneralPage
//

LFStorePropertiesGeneralPage::LFStorePropertiesGeneralPage(LFStoreDescriptor* pStore)
	: CPropertyPage(IDD_STOREPROPERTIES_GENERAL)
{
	ASSERT(pStore);

	p_Store = pStore;
}

void LFStorePropertiesGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STOREICON, m_Icon);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesGeneralPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL LFStorePropertiesGeneralPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_Icon.SetCoreIcon(LFGetStoreIcon(p_Store));

//	if ((p_Store->StoreMode!=LFStoreModeHybrid) && (p_Store->StoreMode!=LFStoreModeExternal))
//		GetDlgItem(IDC_MAKESEARCHABLE)->ShowWindow(SW_HIDE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
