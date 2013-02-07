
// LFStorePropertiesGeneralPage.cpp: Implementierung der Klasse LFStorePropertiesGeneralPage
//

#include "stdafx.h"
#include "LFStorePropertiesGeneralPage.h"
#include "Resource.h"


// LFStorePropertiesGeneralPage
//

extern LFMessageIDs* MessageIDs;

LFStorePropertiesGeneralPage::LFStorePropertiesGeneralPage(LFStoreDescriptor* pStore, BOOL* pStoreValid)
	: CPropertyPage(IDD_STOREPROPERTIES_GENERAL)
{
	ASSERT(pStore);
	ASSERT(pValid);

	p_Store = pStore;
	p_StoreValid = pStoreValid;
}

void LFStorePropertiesGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STOREICON, m_Icon);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesGeneralPage, CPropertyPage)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesGeneralPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_Icon.SetCoreIcon(LFGetStoreIcon(p_Store));

//	if ((p_Store->StoreMode!=LFStoreModeHybrid) && (p_Store->StoreMode!=LFStoreModeExternal))
//		GetDlgItem(IDC_MAKESEARCHABLE)->ShowWindow(SW_HIDE);

	// Store
	SendMessage(MessageIDs->StoresChanged);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

LRESULT LFStorePropertiesGeneralPage::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CEdit* edit1 = (CEdit*)GetDlgItem(IDC_STORENAME);
	CEdit* edit2 = (CEdit*)GetDlgItem(IDC_STORECOMMENT);

	if (p_StoreValid)
	{
		if (edit1->LineLength()==0)
			edit1->SetWindowText(p_Store->StoreName);
		if (edit2->LineLength()==0)
			edit2->SetWindowText(p_Store->StoreComment);

		edit1->EnableWindow(TRUE);
		edit2->EnableWindow(TRUE);

		WCHAR tmpStr[256];
		LFTimeToString(p_Store->CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);
		LFTimeToString(p_Store->FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);

		GetDlgItem(IDC_LASTSEENCAPTION)->EnableWindow(p_Store->StoreMode!=LFStoreModeInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(p_Store->LastSeen);
	}
	else
	{
		edit1->EnableWindow(FALSE);
		edit2->EnableWindow(FALSE);
	}

	return NULL;
}
