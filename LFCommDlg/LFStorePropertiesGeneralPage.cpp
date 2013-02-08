
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
	ASSERT(pStoreValid);

	p_Store = pStore;
	p_StoreValid = pStoreValid;
}

void LFStorePropertiesGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STOREICON, m_wndIcon);
	DDX_Control(pDX, IDC_STORENAME, m_wndStoreName);
	DDX_Control(pDX, IDC_STORECOMMENT, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_wndMakeDefault);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesGeneralPage, CPropertyPage)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesGeneralPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndIcon.SetCoreIcon(LFGetStoreIcon(p_Store));

	if ((p_Store->StoreMode!=LFStoreModeHybrid) && (p_Store->StoreMode!=LFStoreModeExternal))
		GetDlgItem(IDC_MAKESEARCHABLE)->ShowWindow(SW_HIDE);

	// Store
	SendMessage(MessageIDs->StoresChanged);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

LRESULT LFStorePropertiesGeneralPage::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_StoreValid)
	{
		if (m_wndStoreName.LineLength()==0)
			m_wndStoreName.SetWindowText(p_Store->StoreName);
		if (m_wndStoreComment.LineLength()==0)
			m_wndStoreComment.SetWindowText(p_Store->StoreComment);

		WCHAR tmpStr[256];
		LFTimeToString(p_Store->CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);
		LFTimeToString(p_Store->FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);

		GetDlgItem(IDC_LASTSEENCAPTION)->EnableWindow(p_Store->StoreMode!=LFStoreModeInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(p_Store->LastSeen);

		CHAR* pKey = LFGetDefaultStore();
		if (strcmp(pKey, p_Store->StoreID)==0)
			m_wndMakeDefault.SetCheck(TRUE);

		free(pKey);
	}

	m_wndStoreName.EnableWindow(*p_StoreValid);
	m_wndStoreComment.EnableWindow(*p_StoreValid);
	m_wndMakeDefault.EnableWindow(*p_StoreValid);

	return NULL;
}
