
// LFEditConditionDlg.cpp: Implementierung der Klasse LFEditConditionDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditConditionDlg.h"


// LFEditConditionDlg
//

LFEditConditionDlg::LFEditConditionDlg(CWnd* pParentWnd, LFFilterCondition* pCondition)
	: LFAttributeListDlg(IDD_EDITCONDITION, pParentWnd)
{
	if (pCondition)
	{
		m_Condition = *pCondition;
	}
	else
	{
		m_Condition.Compare = LFFilterCompareContains;
		LFInitVariantData(m_Condition.VData, LFAttrFileName);
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
		m_Condition.VData = m_wndEdit.m_VData;
	}
}

void LFEditConditionDlg::TestAttribute(ATTRIBUTE Attr, BOOL& Add, BOOL& Check)
{
	Add = (LFGetApp()->m_Attributes[Attr].AttrProperties.DefaultView!=(UINT)-1);
	Check = FALSE;
}

BOOL LFEditConditionDlg::InitDialog()
{
	LFAttributeListDlg::InitDialog();

	// Bedingung
	m_wndEdit.SetInitialData(m_Condition.VData);

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_COMPAREATTRIBUTE, FALSE, m_Condition.VData.Attr);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFEditConditionDlg, LFAttributeListDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_COMPAREATTRIBUTE, OnItemChanged)
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

void LFEditConditionDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
	{
		const INT Index = m_wndAttribute.GetNextItem(-1, LVNI_SELECTED);
		if (Index!=-1)
		{
			const ATTRIBUTE Attr = (UINT)m_wndAttribute.GetItemData(Index);

			SetCompareComboBox(&m_wndCompare, Attr, m_Condition.Compare);
			m_wndEdit.SetAttribute(Attr);
		}
	}
}

LRESULT LFEditConditionDlg::OnPropertyChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	GetDlgItem(IDOK)->EnableWindow(!m_wndEdit.IsNullData());

	return NULL;
}
