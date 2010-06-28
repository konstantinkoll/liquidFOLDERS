#include "StdAfx.h"
#include "CAttributeListDialog.h"
#include "LFCore.h"

static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	return wcscmp(theApp.m_Attributes[(int)lParam1]->Name, theApp.m_Attributes[(int)lParam2]->Name);
}


// CAttributeListDialog
//

CAttributeListDialog::CAttributeListDialog(UINT nIdTemplate, CWnd* pParentWnd)
	: CDialog(nIdTemplate, pParentWnd)
{
	m_pAttributeIcons = NULL;
}

CAttributeListDialog::~CAttributeListDialog()
{
	if (m_pAttributeIcons)
		delete m_pAttributeIcons;
}

void CAttributeListDialog::PopulateListCtrl(int nId, UINT mode, UINT context, LFViewParameters* vp)
{
	CListCtrl* l = (CListCtrl*)GetDlgItem(nId);
	UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS;
	if (mode==ALD_Mode_ShowAttributes)
		dwExStyle |= LVS_EX_CHECKBOXES;
	l->SetExtendedStyle(l->GetExtendedStyle() | dwExStyle);

	m_pAttributeIcons = new CImageListTransparent();
	m_pAttributeIcons->CreateFromResource(IDB_RIBBONVIEW_16, 21, 43);
	l->SetImageList(m_pAttributeIcons, LVSIL_SMALL);

	const UINT iconPosition[] = { LFAttrFileName, LFAttrTitle, 0xFFFFFFFF, LFAttrCreationTime, LFAttrFileTime,
		LFAttrRecordingTime, LFAttrDeleteTime, LFAttrDueTime, LFAttrDoneTime, LFAttrLocationName,
		LFAttrLocationIATA, LFAttrLocationGPS, LFAttrRating, LFAttrRoll, LFAttrArtist, LFAttrComment,
		LFAttrDuration, LFAttrLanguage, LFAttrResolution, LFAttrHeight, LFAttrWidth, LFAttrAspectRatio, LFAttrTags,
		LFAttrStoreID };

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		for(UINT a=0; a<LFAttributeCount; a++)
		{
			bool add = theApp.m_Contexts[context]->AllowedAttributes->IsSet(a);
			bool check = FALSE;
			switch (mode)
			{
			case ALD_Mode_ShowAttributes:
				add &= !theApp.m_Attributes[a]->AlwaysVisible;
				check = (vp->ColumnWidth[a]>0);
				break;
			case ALD_Mode_SortAttribute:
				add &= theApp.m_Attributes[a]->Sortable;
				lvi.mask |= LVIF_STATE;
				break;
			}

			if (add)
			{
				lvi.lParam = (LPARAM)a;
				lvi.pszText = theApp.m_Attributes[a]->Name;
				lvi.iImage = -1;
				for (UINT b=0; b<sizeof(iconPosition)/sizeof(UINT); b++)
					if (iconPosition[b]==a)
					{
						lvi.iImage = b;
						break;
					}
				l->SetCheck(l->InsertItem(&lvi), check);
			}
		}
	l->SortItems(MyCompareProc, 0);
	l->SetColumnWidth(0, LVSCW_AUTOSIZE);

	int select = 0;
	if (mode==ALD_Mode_SortAttribute)
		for (int a=0; a<l->GetItemCount(); a++)
			if (l->GetItemData(a)==vp->SortBy)
			{
				select = a;
				break;
			}
	l->SetItemState(select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}
