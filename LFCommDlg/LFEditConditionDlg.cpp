
// LFEditConditionDlg.cpp: Implementierung der Klasse LFEditConditionDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditConditionDlg.h"


// LFEditConditionDlg
//

LFEditConditionDlg::LFEditConditionDlg(CWnd* pParentWnd, CHAR* StoreID, LFFilterCondition* pCondition)
	: LFAttributeListDlg(IDD_EDITCONDITION, pParentWnd)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");

	if (pCondition)
	{
		m_Condition = *pCondition;
	}
	else
	{
		m_Condition.Compare = LFFilterCompareContains;
		m_Condition.AttrData.Attr = LFAttrFileName;
		LFGetNullVariantData(&m_Condition.AttrData);
	}
}

void LFEditConditionDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAttributeListDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMPAREATTRIBUTE, m_wndAttribute);
	DDX_Control(pDX, IDC_COMPARE, m_wndCompare);
	DDX_Control(pDX, IDC_PROPERTY, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
		m_Condition.Compare = (UCHAR)m_wndCompare.GetItemData(m_wndCompare.GetCurSel());
		m_Condition.AttrData = m_wndEdit.m_Data;
	}
}

void LFEditConditionDlg::TestAttribute(UINT Attr, BOOL& add, BOOL& check)
{
	add = (Attr!=LFAttrFileID) && (Attr!=LFAttrStoreID) && (Attr!=LFAttrDescription) && (LFGetApp()->m_Attributes[Attr].Type!=LFTypeFlags);
	check = FALSE;
}


BEGIN_MESSAGE_MAP(LFEditConditionDlg, LFAttributeListDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_COMPAREATTRIBUTE, OnItemChanged)
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

BOOL LFEditConditionDlg::OnInitDialog()
{
	LFAttributeListDlg::OnInitDialog();

	// Bedingung
	m_wndEdit.SetData(&m_Condition.AttrData);
	if (m_StoreID[0]!='\0')
		m_wndEdit.SetStore(m_StoreID);

	// Attribut-Liste f�llen
	PopulateListCtrl(IDC_COMPAREATTRIBUTE, FALSE, m_Condition.AttrData.Attr);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFEditConditionDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
	{
		INT Index = m_wndAttribute.GetNextItem(-1, LVNI_SELECTED);
		if (Index!=-1)
		{
			UINT Attr = (UINT)m_wndAttribute.GetItemData(Index);
			SetCompareComboBox(&m_wndCompare, Attr, m_Condition.Compare);
			m_wndEdit.SetAttribute(Attr);
		}
	}
}

LRESULT LFEditConditionDlg::OnPropertyChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	GetDlgItem(IDOK)->EnableWindow(m_wndEdit.m_IsValid && !m_wndEdit.m_IsEmpty);

	return NULL;
}