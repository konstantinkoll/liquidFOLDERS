
// LFEditHashtagsDlg.cpp: Implementierung der Klasse LFEditHashtagsDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditHashtagsDlg.h"


// LFEditHashtagsDlg
//

LFEditHashtagsDlg::LFEditHashtagsDlg(const CString& Hashtags, CHAR* pStoreID, CWnd* pParentWnd)
	: LFDialog(IDD_EDITHASHTAGS, pParentWnd)
{
	ASSERT(pStoreID);

	m_Hashtags = Hashtags;
	strcpy_s(m_StoreID, LFKeySize, pStoreID);
}

void LFEditHashtagsDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ASSIGNEDHASHTAGS, m_wndAssignedHashtags);
	DDX_Control(pDX, IDC_HASHTAGSFROMALLSTORES, m_wndHashtagsFromAllStores);
	DDX_Control(pDX, IDC_NEWHASHTAGS, m_wndNewHashtags);

	if (pDX->m_bSaveAndValidate)
	{
		LFGetApp()->WriteInt(_T("ShowHashtagsFromAllStores"), m_wndHashtagsFromAllStores.GetCheck());

		m_wndNewHashtags.GetWindowText(m_Hashtags);

		for (INT a=0; a<m_wndAssignedHashtags.GetItemCount(); a++)
			if (m_wndAssignedHashtags.GetCheck(a))
			{
				CString Hashtag(m_wndAssignedHashtags.GetItemText(a, 0));
				if (Hashtag.FindOneOf(_T(" ,:;|"))!=-1)
					Hashtag = _T("\"")+Hashtag+_T("\"");

				if (!m_Hashtags.IsEmpty())
					m_Hashtags += _T(" ");

				m_Hashtags += Hashtag;
			}
	}
}

BOOL LFEditHashtagsDlg::InitDialog()
{
	// Assigned hashtags
	m_wndAssignedHashtags.SetExtendedStyle(m_wndAssignedHashtags.GetExtendedStyle() | LVS_EX_CHECKBOXES);
	m_wndAssignedHashtags.AddColumn(0);
	m_wndAssignedHashtags.AddColumn(1);

	m_wndHashtagsFromAllStores.SetCheck(LFGetApp()->GetInt(_T("ShowHashtagsFromAllStores"), FALSE));
	m_wndHashtagsFromAllStores.EnableWindow(m_StoreID[0]);

	OnUpdateAssignedHashtags();

	// New hashtags
	m_wndNewHashtags.SetLimitText(255);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFEditHashtagsDlg, LFDialog)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, IDC_ASSIGNEDHASHTAGS, OnRequestTooltipData)
	ON_BN_CLICKED(IDC_HASHTAGSFROMALLSTORES, OnUpdateAssignedHashtags)
END_MESSAGE_MAP()

void LFEditHashtagsDlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		m_wndAssignedHashtags.GetItemText(pTooltipData->Item, 1, pTooltipData->Hint, 4096);

		*pResult = TRUE;
	}
	else
	{
		*pResult = FALSE;
	}
}

void LFEditHashtagsDlg::OnUpdateAssignedHashtags()
{
	CWaitCursor csr;

	LFFilter* pFilter = LFAllocFilter();
	pFilter->Options.IgnoreSlaves = TRUE;

	if ((!m_StoreID[0]) || (m_wndHashtagsFromAllStores.GetCheck()))
	{
		pFilter->Mode = LFFilterModeSearch;
	}
	else
	{
		pFilter->Mode = LFFilterModeDirectoryTree;
		strcpy_s(pFilter->StoreID, LFKeySize, m_StoreID);
	}

	LFSearchResult* pRawFiles = LFQuery(pFilter);
	LFSearchResult* pCookedFiles = LFGroupSearchResult(pRawFiles, LFAttrHashtags, FALSE, TRUE, pFilter);
	LFFreeFilter(pFilter);

	// Update list
	m_wndAssignedHashtags.DeleteAllItems();

	CRect rectClient;
	m_wndAssignedHashtags.GetClientRect(rectClient);
	m_wndAssignedHashtags.SetColumnWidth(0, rectClient.Width()/4);

	for (UINT a=0; a<pCookedFiles->m_ItemCount; a++)
	{
		LFItemDescriptor* pItemDescriptor = (*pCookedFiles)[a];
		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->AggregateCount))
		{
			INT Index = m_wndAssignedHashtags.InsertItem(a, pItemDescriptor->CoreAttributes.FileName);

			m_wndAssignedHashtags.SetItemText(Index, 1, pItemDescriptor->Description);

			if (LFContainsHashtag(m_Hashtags.GetBuffer(), pItemDescriptor->CoreAttributes.FileName))
				m_wndAssignedHashtags.SetCheck(Index);
		}
	}

	LFFreeSearchResult(pCookedFiles);
	LFFreeSearchResult(pRawFiles);
}
