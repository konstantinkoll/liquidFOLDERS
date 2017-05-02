
// ChooseDetailsDlg.cpp: Implementierung der Klasse ChooseDetailsDlg
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"


// ChooseDetailsDlg
//

ChooseDetailsDlg::ChooseDetailsDlg(UINT Context, CWnd* pParentWnd)
	: LFAttributeListDlg(IDD_CHOOSEDETAILS, pParentWnd)
{
	p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context=Context];
}

void ChooseDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAttributeListDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VIEWATTRIBUTES, m_wndAttributes);

	if (pDX->m_bSaveAndValidate)
	{
		// Width and order
		INT OldWidth[LFAttributeCount];
		memcpy(&OldWidth, &p_ContextViewSettings->ColumnWidth, sizeof(OldWidth));
		ZeroMemory(&p_ContextViewSettings->ColumnWidth, sizeof(p_ContextViewSettings->ColumnWidth));

		UINT Index = 0;

		for (INT a=0; a<m_wndAttributes.GetItemCount(); a++)
		{
			const UINT Attr = (UINT)m_wndAttributes.GetItemData(a);

			if (m_wndAttributes.GetCheck(a) || theApp.m_Attributes[Attr].AttrProperties.AlwaysShow)
			{
				p_ContextViewSettings->ColumnWidth[Attr] = OldWidth[Attr] ? OldWidth[Attr] : theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth;
				p_ContextViewSettings->ColumnOrder[Index++] = Attr;
			}
			else
			{
				p_ContextViewSettings->ColumnWidth[Attr] = 0;
			}
		}

		// Order
		for (INT a=0; a<LFAttributeCount; a++)
			if (!p_ContextViewSettings->ColumnWidth[a])
				p_ContextViewSettings->ColumnOrder[Index++] = a;
	}
}

void ChooseDetailsDlg::TestAttribute(UINT Attr, BOOL& Add, BOOL& Check)
{
	Add = theApp.IsAttributeAvailable(m_Context, Attr);
	Check = (p_ContextViewSettings->ColumnWidth[Attr]!=0);
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
	m_wndAttributes.GetItem(&i1);

	TCHAR text2[256];
	LVITEM i2;
	ZeroMemory(&i2, sizeof(LVITEM));
	i2.pszText = text2;
	i2.cchTextMax = sizeof(text2)/sizeof(TCHAR);
	i2.iItem = NewPos;
	i2.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	m_wndAttributes.GetItem(&i2);

	INT iItem = i1.iItem;
	i1.iItem = i2.iItem;
	i2.iItem = iItem;

	m_wndAttributes.SetItem(&i1);
	m_wndAttributes.SetItem(&i2);

	BOOL Check1 = m_wndAttributes.GetCheck(FocusItem);
	BOOL Check2 = m_wndAttributes.GetCheck(NewPos);
	m_wndAttributes.SetCheck(FocusItem, Check2);
	m_wndAttributes.SetCheck(NewPos, Check1);

	m_wndAttributes.SetItemState(NewPos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
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
	PrepareListCtrl(&m_wndAttributes, TRUE);

	for (UINT a=0; a<LFAttributeCount; a++)
		if (p_ContextViewSettings->ColumnWidth[p_ContextViewSettings->ColumnOrder[a]])
			AddAttribute(&m_wndAttributes, p_ContextViewSettings->ColumnOrder[a]);

	for (UINT a=0; a<LFAttributeCount; a++)
		if (!p_ContextViewSettings->ColumnWidth[p_ContextViewSettings->ColumnOrder[a]])
			AddAttribute(&m_wndAttributes, p_ContextViewSettings->ColumnOrder[a]);

	FinalizeListCtrl(&m_wndAttributes, -1, FALSE);

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

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		GetDlgItem(IDC_MOVEUP)->EnableWindow(pNMListView->iItem>0);
		GetDlgItem(IDC_MOVEDOWN)->EnableWindow(pNMListView->iItem<m_wndAttributes.GetItemCount()-1);
	}

	*pResult = 0;
}

void ChooseDetailsDlg::OnMoveUp()
{
	INT Index = m_wndAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index>0)
		SwapItems(Index, Index-1);
}

void ChooseDetailsDlg::OnMoveDown()
{
	INT Index = m_wndAttributes.GetNextItem(-1, LVIS_SELECTED);
	if (Index<m_wndAttributes.GetItemCount()-1)
		SwapItems(Index, Index+1);
}

void ChooseDetailsDlg::OnCheckAll()
{
	for (INT a=0; a<m_wndAttributes.GetItemCount(); a++)
		m_wndAttributes.SetCheck(a);
}

void ChooseDetailsDlg::OnUncheckAll()
{
	for (INT a=0; a<m_wndAttributes.GetItemCount(); a++)
		m_wndAttributes.SetCheck(a, FALSE);
}
