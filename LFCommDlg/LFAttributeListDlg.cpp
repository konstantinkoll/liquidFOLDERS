
// LFAttributeListDlg.cpp: Implementierung der Klasse LFAttributeListDlg
//

#include "stdafx.h"
#include "LFAttributeListDlg.h"
#include "resource.h"


static INT CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	LFApplication* pApp = (LFApplication*)AfxGetApp();
	return wcscmp(pApp->m_Attributes[(INT)lParam1]->Name, pApp->m_Attributes[(INT)lParam2]->Name);
}


// LFAttributeListDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFAttributeListDlg::LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{
	p_App = (LFApplication*)AfxGetApp();
}

void LFAttributeListDlg::TestAttribute(UINT /*attr*/, BOOL& add, BOOL& check)
{
	add = TRUE;
	check = FALSE;
}

void LFAttributeListDlg::PrepareListCtrl(CListCtrl* li, BOOL check)
{
	const UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS | (check ? LVS_EX_CHECKBOXES : 0);
	li->SetExtendedStyle(li->GetExtendedStyle() | dwExStyle);
	li->ModifyStyle(0, LVS_SHAREIMAGELISTS);

	li->SetImageList(&m_AttributeIcons, LVSIL_SMALL);
}

void LFAttributeListDlg::PrepareListCtrl(INT nID, BOOL check)
{
	PrepareListCtrl((CListCtrl*)GetDlgItem(nID), check);
}

void LFAttributeListDlg::AddAttribute(CListCtrl* li, UINT attr)
{
	BOOL add;
	BOOL check;
	TestAttribute(attr , add, check);
	if (!add)
		return;

	static const UINT IconPosition[] = { LFAttrFileName, LFAttrTitle, LFAttrCreationTime, LFAttrAddTime,
		LFAttrFileTime, LFAttrRecordingTime, LFAttrDeleteTime, LFAttrDueTime, LFAttrDoneTime, LFAttrLocationName,
		LFAttrLocationIATA, LFAttrLocationGPS, LFAttrRating, LFAttrRoll, LFAttrArtist, LFAttrComment,
		LFAttrDuration, LFAttrLanguage, LFAttrDimension, LFAttrWidth, LFAttrHeight, LFAttrAspectRatio, LFAttrTags,
		LFAttrStoreID, LFAttrAlbum, LFAttrPriority, LFAttrURL, LFAttrISBN, LFAttrRecordingEquipment, LFAttrCustomer };

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.lParam = (LPARAM)attr;
	lvi.pszText = p_App->m_Attributes[attr]->Name;
	lvi.iImage = -1;
	lvi.iItem = li->GetItemCount();

	for (UINT a=0; a<sizeof(IconPosition)/sizeof(UINT); a++)
		if (IconPosition[a]==attr)
		{
			lvi.iImage = a;
			break;
		}

	li->SetCheck(li->InsertItem(&lvi), check);
}

void LFAttributeListDlg::AddAttribute(UINT nID, UINT attr)
{
	AddAttribute((CListCtrl*)GetDlgItem(nID), attr);
}

void LFAttributeListDlg::FinalizeListCtrl(CListCtrl* li, INT focus, BOOL sort)
{
	li->SetColumnWidth(0, LVSCW_AUTOSIZE);

	if (sort)
		li->SortItems(MyCompareProc, 0);

	INT select = 0;
	if (focus!=-1)
		for (UINT a=0; a<(UINT)li->GetItemCount(); a++)
			if ((INT)li->GetItemData(a)==focus)
			{
				select = a;
				break;
			}
	li->SetItemState(select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void LFAttributeListDlg::FinalizeListCtrl(UINT nID, INT focus, BOOL sort)
{
	FinalizeListCtrl((CListCtrl*)GetDlgItem(nID), focus, sort);
}

void LFAttributeListDlg::PopulateListCtrl(CListCtrl* li, BOOL check, INT focus, BOOL sort)
{
	PrepareListCtrl(li, check);

	for (UINT a=0; a<LFAttributeCount; a++)
		AddAttribute(li, a);

	FinalizeListCtrl(li, focus, sort);
}

void LFAttributeListDlg::PopulateListCtrl(INT nID, BOOL check, INT focus, BOOL sort)
{
	PopulateListCtrl((CListCtrl*)GetDlgItem(nID), check, focus, sort);
}


BEGIN_MESSAGE_MAP(LFAttributeListDlg, CDialog)
END_MESSAGE_MAP()

BOOL LFAttributeListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_AttributeIcons.Create(IDB_ATTRIBUTEICONS, LFCommDlgDLL.hResource);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
