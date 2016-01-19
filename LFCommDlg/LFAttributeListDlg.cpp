
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

void LFAttributeListDlg::PrepareListCtrl(CExplorerList* pExplorerList, BOOL Check)
{
	if (Check)
		pExplorerList->SetExtendedStyle(pExplorerList->GetExtendedStyle() | LVS_EX_CHECKBOXES);

	pExplorerList->AddColumn(0);

	pExplorerList->ModifyStyle(0, LVS_SHAREIMAGELISTS);
	pExplorerList->SetImageList(&m_AttributeIcons, LVSIL_SMALL);
}

void LFAttributeListDlg::PrepareListCtrl(INT nID, BOOL Check)
{
	PrepareListCtrl((CExplorerList*)GetDlgItem(nID), Check);
}

void LFAttributeListDlg::AddAttribute(CExplorerList* pExplorerList, UINT Attr)
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
	lvi.iItem = pExplorerList->GetItemCount();

	pExplorerList->SetCheck(pExplorerList->InsertItem(&lvi), Check);
}

void LFAttributeListDlg::AddAttribute(UINT nID, UINT Attr)
{
	AddAttribute((CExplorerList*)GetDlgItem(nID), Attr);
}

void LFAttributeListDlg::FinalizeListCtrl(CExplorerList* pExplorerList, INT Focus, BOOL Sort)
{
	pExplorerList->SetColumnWidth(0, LVSCW_AUTOSIZE);

	if (Sort)
		pExplorerList->SortItems(MyCompareProc, 0);

	INT Select = 0;
	if (Focus!=-1)
		for (UINT a=0; a<(UINT)pExplorerList->GetItemCount(); a++)
			if ((INT)pExplorerList->GetItemData(a)==Focus)
			{
				Select = a;
				break;
			}

	pExplorerList->SetItemState(Select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	if (pExplorerList->GetView()==LV_VIEW_DETAILS)
		pExplorerList->SetColumnWidth(0, LVSCW_AUTOSIZE);
}

void LFAttributeListDlg::FinalizeListCtrl(UINT nID, INT Focus, BOOL Sort)
{
	FinalizeListCtrl((CExplorerList*)GetDlgItem(nID), Focus, Sort);
}

void LFAttributeListDlg::PopulateListCtrl(CExplorerList* pExplorerList, BOOL Check, INT Focus, BOOL Sort)
{
	PrepareListCtrl(pExplorerList, Check);

	for (UINT a=0; a<LFAttributeCount; a++)
		AddAttribute(pExplorerList, a);

	FinalizeListCtrl(pExplorerList, Focus, Sort);
}

void LFAttributeListDlg::PopulateListCtrl(INT nID, BOOL Check, INT Focus, BOOL Sort)
{
	PopulateListCtrl((CExplorerList*)GetDlgItem(nID), Check, Focus, Sort);
}

BOOL LFAttributeListDlg::InitDialog()
{
	LFGetApp()->m_SmallAttributeIcons.Load(IDB_ATTRIBUTEICONS_16, 16);
	m_AttributeIcons.Attach(LFGetApp()->m_SmallAttributeIcons.ExtractImageList());

	return TRUE;
}
