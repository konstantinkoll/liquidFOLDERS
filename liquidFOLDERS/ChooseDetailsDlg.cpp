
// ChooseDetailsDlg.cpp: Implementierung der Klasse ChooseDetailsDlg
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"


// ChooseDetailsDlg
//

ChooseDetailsDlg::ChooseDetailsDlg(UINT Context, CWnd* pParentWnd)
	: LFAttributeListDlg(IDD_CHOOSEDETAILS, pParentWnd)
{
	p_View = &theApp.m_Views[Context];
	m_Context = Context;
}

void ChooseDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAttributeListDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VIEWATTRIBUTES, m_ShowAttributes);

	if (pDX->m_bSaveAndValidate)
	{
		// Breite
		INT OldWidth[LFAttributeCount];
		memcpy(&OldWidth, &p_View->ColumnWidth, sizeof(OldWidth));
		ZeroMemory(&p_View->ColumnWidth, sizeof(p_View->ColumnWidth));

		for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		{
			UINT Attr = (UINT)m_ShowAttributes.GetItemData(a);
			p_View->ColumnWidth[Attr] = m_ShowAttributes.GetCheck(a) ? OldWidth[Attr] ? OldWidth[Attr] : theApp.m_Attributes[Attr].RecommendedWidth : 0;
		}

		// Reihenfolge
		p_View->ColumnOrder[0] = 0;
		p_View->ColumnWidth[0] = OldWidth[0];
		UINT cnt = 1;

		for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
			if (m_ShowAttributes.GetCheck(a))
				p_View->ColumnOrder[cnt++] = (INT)m_ShowAttributes.GetItemData(a);

		for (INT a=0; a<LFAttributeCount; a++)
			if (!p_View->ColumnWidth[a])
				p_View->ColumnOrder[cnt++] = a;
	}
}

void ChooseDetailsDlg::TestAttribute(UINT Attr, BOOL& Add, BOOL& Check)
{
	Add = LFIsAttributeAllowed(theApp.m_Contexts[m_Context], Attr) && (!theApp.m_Attributes[Attr].AlwaysVisible);
	Check = (p_View->ColumnWidth[Attr]);
}

void ChooseDetailsDlg::SwapItems(INT FocusItem, INT NewPos)
{
	TCHAR text1[256];
	LVITEM i1;
	ZeroMemory(&i1, sizeof(LVITEM));
	i1.pszText = text1;
	i1.cchTextMax = sizeof(text1)/sizeof(TCHAR);
	i1.iItem = FocusItem;
	i1.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	m_ShowAttributes.GetItem(&i1);

	TCHAR text2[256];
	LVITEM i2;
	ZeroMemory(&i2, sizeof(LVITEM));
	i2.pszText = text2;
	i2.cchTextMax = sizeof(text2)/sizeof(TCHAR);
	i2.iItem = NewPos;
	i2.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	m_ShowAttributes.GetItem(&i2);

	INT iItem = i1.iItem;
	i1.iItem = i2.iItem;
	i2.iItem = iItem;

	m_ShowAttributes.SetItem(&i1);
	m_ShowAttributes.SetItem(&i2);

	BOOL Check1 = m_ShowAttributes.GetCheck(FocusItem);
	BOOL Check2 = m_ShowAttributes.GetCheck(NewPos);
	m_ShowAttributes.SetCheck(FocusItem, Check2);
	m_ShowAttributes.SetCheck(NewPos, Check1);

	m_ShowAttributes.SetItemState(NewPos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

BOOL ChooseDetailsDlg::InitDialog()
{
	LFAttributeListDlg::InitDialog();

	// Titelleiste
	CString Text;
	GetWindowText(Text);

	CString Caption;
	Caption.Format(Text, theApp.m_Contexts[m_Context].Name);
	SetWindowText(Caption);

	// Kontrollelemente einstellen
	PrepareListCtrl(&m_ShowAttributes, TRUE);

	for (UINT a=0; a<LFAttributeCount; a++)
		if (p_View->ColumnWidth[p_View->ColumnOrder[a]])
			AddAttribute(&m_ShowAttributes, p_View->ColumnOrder[a]);

	for (UINT a=0; a<LFAttributeCount; a++)
		if (!p_View->ColumnWidth[p_View->ColumnOrder[a]])
			AddAttribute(&m_ShowAttributes, p_View->ColumnOrder[a]);

	FinalizeListCtrl(&m_ShowAttributes, -1, FALSE);

	return TRUE;
}


BEGIN_MESSAGE_MAP(ChooseDetailsDlg, LFAttributeListDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWATTRIBUTES, OnSelectionChange)
	ON_COMMAND(IDC_MOVEUP, OnMoveUp)
	ON_COMMAND(IDC_MOVEDOWN, OnMoveDown)
	ON_COMMAND(IDC_CHECKALL, OnCheckAll)
	ON_COMMAND(IDC_UNCHECKALL, OnUncheckAll)
END_MESSAGE_MAP()

void ChooseDetailsDlg::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	INT Index = (INT)pNMListView->iItem;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		GetDlgItem(IDC_MOVEUP)->EnableWindow(m_ShowAttributes.IsWindowEnabled() && (Index>0));
		GetDlgItem(IDC_MOVEDOWN)->EnableWindow(m_ShowAttributes.IsWindowEnabled() && (Index<m_ShowAttributes.GetItemCount()-1));
	}

	*pResult = 0;
}

void ChooseDetailsDlg::OnMoveUp()
{
	INT Index = m_ShowAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index>0)
		SwapItems(Index, Index-1);
}

void ChooseDetailsDlg::OnMoveDown()
{
	INT Index = m_ShowAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index<m_ShowAttributes.GetItemCount()-1)
		SwapItems(Index, Index+1);
}

void ChooseDetailsDlg::OnCheckAll()
{
	for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		m_ShowAttributes.SetCheck(a);
}

void ChooseDetailsDlg::OnUncheckAll()
{
	for (INT a=0; a<m_ShowAttributes.GetItemCount(); a++)
		m_ShowAttributes.SetCheck(a, FALSE);
}
