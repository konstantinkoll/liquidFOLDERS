#include "StdAfx.h"
#include "CAttributeListDialog.h"
#include "LFCore.h"

static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	return wcscmp(theApp.m_Attributes[(int)lParam1]->Name, theApp.m_Attributes[(int)lParam2]->Name);
}

void AddAttribute(CListCtrl* l, UINT attr, BOOL check)
{
	static const UINT iconPosition[] = { LFAttrFileName, LFAttrTitle, 0xFFFFFFFF, LFAttrCreationTime, LFAttrAddTime,
		LFAttrFileTime, LFAttrRecordingTime, LFAttrDeleteTime, LFAttrDueTime, LFAttrDoneTime, LFAttrLocationName,
		LFAttrLocationIATA, LFAttrLocationGPS, LFAttrRating, LFAttrRoll, LFAttrArtist, LFAttrComment,
		LFAttrDuration, LFAttrLanguage, LFAttrResolution, LFAttrHeight, LFAttrWidth, LFAttrAspectRatio, LFAttrTags,
		LFAttrStoreID };

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.lParam = (LPARAM)attr;
	lvi.pszText = theApp.m_Attributes[attr]->Name;
	lvi.iImage = -1;
	lvi.iItem = l->GetItemCount();

	for (UINT b=0; b<sizeof(iconPosition)/sizeof(UINT); b++)
		if (iconPosition[b]==attr)
		{
			lvi.iImage = b;
			break;
		}

	l->SetCheck(l->InsertItem(&lvi), check);
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
	if (mode!=ALD_Mode_SortAttribute)
		dwExStyle |= LVS_EX_CHECKBOXES;
	l->SetExtendedStyle(l->GetExtendedStyle() | dwExStyle);

	m_pAttributeIcons = new CImageListTransparent();
	m_pAttributeIcons->CreateFromResource(IDB_RIBBONVIEW_16, 21, 44);
	l->SetImageList(m_pAttributeIcons, LVSIL_SMALL);

		for (UINT a=0; a<LFAttributeCount; a++)
		{
			int attr = 0;
			if (mode==ALD_Mode_ChooseDetails)
			{
				int cnt = 0;
				for (UINT b=0; b<LFAttributeCount; b++)
					if (vp->ColumnWidth[b])
						if ((cnt++)==vp->ColumnOrder[a])
						{
							attr = b;
							break;
						}
				if (attr==-1)
					continue;
			}
			else
			{
				attr = a;
			}
			
			bool add = theApp.m_Contexts[context]->AllowedAttributes->IsSet(attr);
			BOOL check = FALSE;
			switch (mode)
			{
			case ALD_Mode_ShowAttributes:
				check = vp->ColumnWidth[attr];
				add &= !theApp.m_Attributes[attr]->AlwaysVisible;
				break;
			case ALD_Mode_SortAttribute:
				add &= theApp.m_Attributes[attr]->Sortable;
				break;
			case ALD_Mode_ChooseDetails:
				add &= (!theApp.m_Attributes[attr]->AlwaysVisible) && (vp->ColumnWidth[attr]);
				check = add;
				break;
			}

			if (add)
				AddAttribute(l, attr, check);
		}

	if (mode!=ALD_Mode_ChooseDetails)
	{
		l->SortItems(MyCompareProc, 0);
	}
	else
		for (UINT a=0; a<LFAttributeCount; a++)
			if ((vp->ColumnWidth[a]==0) && (!theApp.m_Attributes[a]->AlwaysVisible) && (theApp.m_Contexts[context]->AllowedAttributes->IsSet(a)))
				AddAttribute(l, a, FALSE);

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
