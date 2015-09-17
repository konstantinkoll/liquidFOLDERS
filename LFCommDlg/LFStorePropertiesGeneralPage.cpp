
// LFStorePropertiesGeneralPage.cpp: Implementierung der Klasse LFStorePropertiesGeneralPage
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFStorePropertiesGeneralPage.h"


// LFStorePropertiesGeneralPage
//

LFStorePropertiesGeneralPage::LFStorePropertiesGeneralPage(LFStoreDescriptor* pStore, BOOL* pStoreValid)
	: CPropertyPage()
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
	DDX_Control(pDX, IDC_COMMENTS, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_wndMakeDefault);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesGeneralPage, CPropertyPage)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StatisticsChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesGeneralPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndIcon.SetCoreIcon(LFGetStoreIcon(p_Store));

	if ((p_Store->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
	{
		m_wndMakeSearchable.ShowWindow(SW_HIDE);
	}
	else
	{
		m_wndMakeSearchable.SetCheck((p_Store->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid);
	}

	// Store
	SendMessage(LFGetApp()->p_MessageIDs->StoresChanged);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

LRESULT LFStorePropertiesGeneralPage::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_StoreValid)
	{
		if (m_wndStoreName.LineLength()==0)
			m_wndStoreName.SetWindowText(p_Store->StoreName);
		if (m_wndStoreComment.LineLength()==0)
			m_wndStoreComment.SetWindowText(p_Store->Comments);

		WCHAR tmpStr[256];
		LFCombineFileCountSize(p_Store->FileCount[LFContextAllFiles], p_Store->FileSize[LFContextAllFiles], tmpStr, 256);
		GetDlgItem(IDC_CONTENTS)->SetWindowText(tmpStr);

		LFTimeToString(p_Store->CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);
		LFTimeToString(p_Store->FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);

		GetDlgItem(IDC_LASTSEENCAPTION)->EnableWindow((p_Store->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(p_Store->LastSeen);

		CHAR StoreID[LFKeySize];
		if (LFGetDefaultStore(StoreID)==LFOk)
			m_wndMakeDefault.SetCheck(strcmp(p_Store->StoreID, StoreID)==0);
	}

	m_wndStoreName.EnableWindow(*p_StoreValid);
	m_wndStoreComment.EnableWindow(*p_StoreValid);
	m_wndMakeDefault.EnableWindow(*p_StoreValid);
	m_wndMakeSearchable.EnableWindow(*p_StoreValid);

	return NULL;
}
