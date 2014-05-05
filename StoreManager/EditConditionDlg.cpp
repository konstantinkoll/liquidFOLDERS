
// EditConditionDlg.cpp: Implementierung der Klasse EditConditionDlg
//

#include "stdafx.h"
#include "EditConditionDlg.h"


// EditConditionDlg
//

EditConditionDlg::EditConditionDlg(CWnd* pParentWnd, CHAR* StoreID, LFFilterCondition* pCondition)
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

void EditConditionDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_COMPAREATTRIBUTE, m_wndAttribute);
	DDX_Control(pDX, IDC_COMPARE, m_wndCompare);
	DDX_Control(pDX, IDC_PROPERTY, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
		m_Condition.Compare = (UCHAR)m_wndCompare.GetItemData(m_wndCompare.GetCurSel());
		m_Condition.AttrData = m_wndEdit.m_Data;
	}
}

void EditConditionDlg::TestAttribute(UINT attr, BOOL& add, BOOL& check)
{
	add = (attr!=LFAttrFileID) && (attr!=LFAttrStoreID) && (attr!=LFAttrDescription) && (theApp.m_Attributes[attr].Type!=LFTypeFlags);
	check = FALSE;
}


BEGIN_MESSAGE_MAP(EditConditionDlg, LFAttributeListDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_COMPAREATTRIBUTE, OnItemChanged)
	ON_MESSAGE(WM_PROPERTYCHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

BOOL EditConditionDlg::OnInitDialog()
{
	LFAttributeListDlg::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_EDITCONDITION);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Bedingung
	m_wndEdit.SetData(&m_Condition.AttrData);
	if (m_StoreID[0]!='\0')
		m_wndEdit.SetStore(m_StoreID);

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_COMPAREATTRIBUTE, FALSE, m_Condition.AttrData.Attr);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditConditionDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
	{
		INT idx = m_wndAttribute.GetNextItem(-1, LVNI_SELECTED);
		if (idx!=-1)
		{
			UINT attr = (UINT)m_wndAttribute.GetItemData(idx);
			SetCompareComboBox(&m_wndCompare, attr, m_Condition.Compare);
			m_wndEdit.SetAttribute(attr);
		}
	}
}

LRESULT EditConditionDlg::OnPropertyChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
	GetDlgItem(IDOK)->EnableWindow(m_wndEdit.m_IsValid && !m_wndEdit.m_IsEmpty);

	return NULL;
}
