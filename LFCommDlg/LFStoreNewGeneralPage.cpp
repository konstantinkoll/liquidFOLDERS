
// LFStoreNewGeneralPage.cpp: Implementierung der Klasse LFStoreNewGeneralPage
//

#include "stdafx.h"
#include "LFStoreNewGeneralPage.h"
#include "Resource.h"


// LFStorePropertiesGeneralPage
//

extern LFMessageIDs* MessageIDs;

void LFStoreNewGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STOREICON, m_wndIcon);
	DDX_Control(pDX, IDC_STORENAME, m_wndStoreName);
	DDX_Control(pDX, IDC_STORECOMMENT, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_wndMakeDefault);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);
}
