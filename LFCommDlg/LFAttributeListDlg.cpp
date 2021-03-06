
// LFAttributeListDlg.cpp: Implementierung der Klasse LFAttributeListDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


INT CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	LPCWSTR* pAttributeNames = (LPCWSTR*)lParamSort;
	ASSERT(pAttributeNames);

	return wcscmp(pAttributeNames[(INT)lParam1], pAttributeNames[(INT)lParam2]);
}


// LFAttributeListDlg
//

LFAttributeListDlg::LFAttributeListDlg(UINT nIDTemplate, CWnd* pParentWnd, ITEMCONTEXT Context)
	: LFDialog(nIDTemplate, pParentWnd)
{
	m_Context = Context;

	for (UINT a=0; a<LFAttributeCount; a++)
		p_AttributeNames[a] = LFGetApp()->GetAttributeName(a, Context);
}

void LFAttributeListDlg::TestAttribute(UINT /*Attr*/, BOOL& Add, BOOL& Check)
{
	Add = TRUE;
	Check = FALSE;
}

void LFAttributeListDlg::PrepareListCtrl(CExplorerList* pExplorerList, BOOL Check)
{
	ASSERT(pExplorerList);

	if (Check)
		pExplorerList->SetExtendedStyle(pExplorerList->GetExtendedStyle() | LVS_EX_CHECKBOXES);

	pExplorerList->AddColumn(0);

	pExplorerList->ModifyStyle(0, LVS_SHAREIMAGELISTS);
	pExplorerList->SetImageList(&LFGetApp()->m_CoreImageListSmall, LVSIL_SMALL);
}

void LFAttributeListDlg::PrepareListCtrl(INT nID, BOOL Check)
{
	PrepareListCtrl((CExplorerList*)GetDlgItem(nID), Check);
}

void LFAttributeListDlg::AddAttribute(CExplorerList* pExplorerList, ATTRIBUTE Attr)
{
	ASSERT(pExplorerList);

	BOOL Add;
	BOOL Check;
	TestAttribute(Attr , Add, Check);

	if (!Add)
		return;

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.lParam = (LPARAM)Attr;
	lvi.pszText = (LPWSTR)p_AttributeNames[Attr];
	lvi.iImage = LFGetApp()->GetAttributeIcon(Attr, m_Context)-1;
	lvi.iItem = pExplorerList->GetItemCount();

	pExplorerList->SetCheck(pExplorerList->InsertItem(&lvi), Check);
}

void LFAttributeListDlg::AddAttribute(UINT nID, ATTRIBUTE Attr)
{
	AddAttribute((CExplorerList*)GetDlgItem(nID), Attr);
}

void LFAttributeListDlg::FinalizeListCtrl(CExplorerList* pExplorerList, INT Focus, BOOL Sort)
{
	ASSERT(pExplorerList);

	pExplorerList->SetColumnWidth(0, LVSCW_AUTOSIZE);

	if (Sort)
		pExplorerList->SortItems(MyCompareProc, (DWORD_PTR)p_AttributeNames);

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
	ASSERT(pExplorerList);

	PrepareListCtrl(pExplorerList, Check);

	for (UINT a=0; a<LFAttributeCount; a++)
		AddAttribute(pExplorerList, a);

	FinalizeListCtrl(pExplorerList, Focus, Sort);
}

void LFAttributeListDlg::PopulateListCtrl(INT nID, BOOL Check, INT Focus, BOOL Sort)
{
	PopulateListCtrl((CExplorerList*)GetDlgItem(nID), Check, Focus, Sort);
}
