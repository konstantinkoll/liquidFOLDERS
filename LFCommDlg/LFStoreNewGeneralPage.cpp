
// LFStoreNewGeneralPage.cpp: Implementierung der Klasse LFStoreNewGeneralPage
//

#include "stdafx.h"
#include "LFStoreNewGeneralPage.h"
#include "Resource.h"


// LFStorePropertiesGeneralPage
//

extern LFMessageIDs* MessageIDs;

LFStoreNewGeneralPage::LFStoreNewGeneralPage(CHAR Volume)
	: CPropertyPage()
{
	m_Volume = Volume;
}

void LFStoreNewGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STOREICON, m_wndIcon);
	DDX_Control(pDX, IDC_STORENAME, m_wndStoreName);
	DDX_Control(pDX, IDC_STORECOMMENT, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_wndMakeDefault);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);
}


BEGIN_MESSAGE_MAP(LFStoreNewGeneralPage, CPropertyPage)
END_MESSAGE_MAP()

BOOL LFStoreNewGeneralPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
