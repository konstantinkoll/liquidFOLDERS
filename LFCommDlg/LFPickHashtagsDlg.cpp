
// LFPickHashtagsDlg.cpp: Implementierung der Klasse LFPickHashtagsDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFPickHashtagsDlg.h"


// LFPickHashtagsDlg
//

LFPickHashtagsDlg::LFPickHashtagsDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, const CString& Hashtags, CWnd* pParentWnd)
	: CAttributePickDlg(Attr, Context, IDD_PICKHASHTAGS, pParentWnd)
{
	ASSERT(LFGetApp()->m_Attributes[Attr].AttrProperties.Type==LFTypeUnicodeArray);

	m_Hashtags = Hashtags;
}

void LFPickHashtagsDlg::DoDataExchange(CDataExchange* pDX)
{
	CAttributePickDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ASSIGNEDHASHTAGS, m_wndAssignedHashtags);
	DDX_Control(pDX, IDC_NEWHASHTAGS, m_wndNewHashtags);

	if (pDX->m_bSaveAndValidate)
	{
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

BOOL LFPickHashtagsDlg::InitDialog()
{
	CAttributePickDlg::InitDialog();

	// Assigned hashtags
	m_wndAssignedHashtags.SetExtendedStyle(m_wndAssignedHashtags.GetExtendedStyle() | LVS_EX_CHECKBOXES);
	m_wndAssignedHashtags.AddColumn(0);
	m_wndAssignedHashtags.AddColumn(1);

	// Update list
	LFSearchResult* pSearchResult = RunQuery();

	CRect rectClient;
	m_wndAssignedHashtags.GetClientRect(rectClient);
	m_wndAssignedHashtags.SetColumnWidth(0, rectClient.Width()/4);

	for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
	{
		const LFItemDescriptor* pItemDescriptor = (*pSearchResult)[a];

		if (LFIsFolder(pItemDescriptor) && pItemDescriptor->AggregateCount)
		{
			const INT Index = m_wndAssignedHashtags.InsertItem(a, pItemDescriptor->CoreAttributes.FileName);

			m_wndAssignedHashtags.SetItemText(Index, 1, pItemDescriptor->Description);

			if (LFContainsHashtag(m_Hashtags, pItemDescriptor->CoreAttributes.FileName))
				m_wndAssignedHashtags.SetCheck(Index);
		}
	}

	// Free search result
	LFFreeSearchResult(pSearchResult);

	// New hashtags
	m_wndNewHashtags.SetLimitText(255);

	return TRUE;
}


/*BEGIN_MESSAGE_MAP(LFPickHashtagsDlg, CAttributePickDlg)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, IDC_ASSIGNEDHASHTAGS, OnRequestTooltipData)
END_MESSAGE_MAP()

void LFPickHashtagsDlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
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
}*/
