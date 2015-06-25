
// LFAttributeListDlg.cpp: Implementierung der Klasse LFAttributeListDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


static INT CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	return wcscmp(LFGetApp()->m_Attributes[(INT)lParam1].Name, LFGetApp()->m_Attributes[(INT)lParam2].Name);
}


// LFAttributeListDlg
//

extern INT GetAttributeIconIndex(UINT Attr);

LFAttributeListDlg::LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd)
	: LFDialog(nIDTemplate, pParentWnd)
{
}

void LFAttributeListDlg::TestAttribute(UINT /*Attr*/, BOOL& Add, BOOL& Check)
{
	Add = TRUE;
	Check = FALSE;
}

void LFAttributeListDlg::PrepareListCtrl(CListCtrl* li, BOOL Check)
{
	const UINT dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS | (Check ? LVS_EX_CHECKBOXES : 0);
	li->SetExtendedStyle(li->GetExtendedStyle() | dwExStyle);
	li->ModifyStyle(0, LVS_SHAREIMAGELISTS);

	li->SetImageList(&m_AttributeIcons, LVSIL_SMALL);
}

void LFAttributeListDlg::PrepareListCtrl(INT nID, BOOL Check)
{
	PrepareListCtrl((CListCtrl*)GetDlgItem(nID), Check);
}

void LFAttributeListDlg::AddAttribute(CListCtrl* li, UINT Attr)
{
	BOOL Add;
	BOOL Check;
	TestAttribute(Attr , Add, Check);
	if (!Add)
		return;

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.lParam = (LPARAM)Attr;
	lvi.pszText = LFGetApp()->m_Attributes[Attr].Name;
	lvi.iImage = GetAttributeIconIndex(Attr);
	lvi.iItem = li->GetItemCount();

	li->SetCheck(li->InsertItem(&lvi), Check);
}

void LFAttributeListDlg::AddAttribute(UINT nID, UINT Attr)
{
	AddAttribute((CListCtrl*)GetDlgItem(nID), Attr);
}

void LFAttributeListDlg::FinalizeListCtrl(CListCtrl* li, INT Focus, BOOL Sort)
{
	li->SetColumnWidth(0, LVSCW_AUTOSIZE);

	if (Sort)
		li->SortItems(MyCompareProc, 0);

	INT select = 0;
	if (Focus!=-1)
		for (UINT a=0; a<(UINT)li->GetItemCount(); a++)
			if ((INT)li->GetItemData(a)==Focus)
			{
				select = a;
				break;
			}

	li->SetItemState(select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void LFAttributeListDlg::FinalizeListCtrl(UINT nID, INT Focus, BOOL Sort)
{
	FinalizeListCtrl((CListCtrl*)GetDlgItem(nID), Focus, Sort);
}

void LFAttributeListDlg::PopulateListCtrl(CListCtrl* li, BOOL Check, INT Focus, BOOL Sort)
{
	PrepareListCtrl(li, Check);

	for (UINT a=0; a<LFAttributeCount; a++)
		AddAttribute(li, a);

	FinalizeListCtrl(li, Focus, Sort);
}

void LFAttributeListDlg::PopulateListCtrl(INT nID, BOOL Check, INT Focus, BOOL Sort)
{
	PopulateListCtrl((CListCtrl*)GetDlgItem(nID), Check, Focus, Sort);
}


BEGIN_MESSAGE_MAP(LFAttributeListDlg, LFDialog)
END_MESSAGE_MAP()

BOOL LFAttributeListDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	m_AttributeIcons.Create(IDB_ATTRIBUTEICONS_16);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
