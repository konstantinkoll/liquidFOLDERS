
// LFStoreNewGeneralPage.cpp: Implementierung der Klasse LFStoreNewGeneralPage
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFStoreNewGeneralPage.h"


// LFStorePropertiesGeneralPage
//

void LFStoreNewGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_STOREICON, m_wndIcon);
	DDX_Control(pDX, IDC_STORENAME, m_wndStoreName);
	DDX_Control(pDX, IDC_STORECOMMENT, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);
}
