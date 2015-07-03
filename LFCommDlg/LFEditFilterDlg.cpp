
// LFEditFilterDlg.cpp: Implementierung der Klasse LFEditFilterDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditConditionDlg.h"
#include "LFSaveFilterDlg.h"


// LFEditFilterDlg
//

LFEditFilterDlg::LFEditFilterDlg(CWnd* pParentWnd, CHAR* StoreID, LFFilter* pFilter)
	: LFDialog(IDD_EDITFILTER, pParentWnd)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");
	p_Filter = pFilter;
}

void LFEditFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ALLSTORES, m_wndAllStores);
	DDX_Control(pDX, IDC_THISSTORE, m_wndThisStore);
	DDX_Control(pDX, IDC_SEARCHTERM, m_wndSearchterm);
	DDX_Control(pDX, IDC_CONDITIONLIST, m_wndList);
}

LFFilter* LFEditFilterDlg::CreateFilter()
{
	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeSearch;
	f->Options.IsPersistent = TRUE;
	strcpy_s(f->StoreID, LFKeySize, m_wndAllStores.GetCheck() ? "" : m_StoreID);
	m_wndSearchterm.GetWindowText(f->Searchterm, 256);

	for (INT a=m_Conditions.m_ItemCount-1; a>=0; a--)
		f->ConditionList = LFAllocFilterCondition(m_Conditions.m_Items[a].Compare, m_Conditions.m_Items[a].AttrData, f->ConditionList);

	return f;
}


BEGIN_MESSAGE_MAP(LFEditFilterDlg, LFDialog)
	ON_BN_CLICKED(IDM_CONDITIONLIST_ADD, OnAddCondition)
	ON_BN_CLICKED(IDOK, OnSave)
	ON_NOTIFY(NM_DBLCLK, IDC_CONDITIONLIST, OnDoubleClick)

	ON_COMMAND(IDM_CONDITION_EDIT, OnEditCondition)
	ON_COMMAND(IDM_CONDITION_DELETE, OnDeleteCondition)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CONDITION_EDIT, IDM_CONDITION_DELETE, OnUpdateCommands)
END_MESSAGE_MAP()

BOOL LFEditFilterDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Store-Namen einsetzen
	BOOL InStore = FALSE;
	if (m_StoreID[0]!='\0')
	{
		LFStoreDescriptor s;
		if (LFGetStoreSettings(m_StoreID, &s)==LFOk)
		{
			InStore = TRUE;

			CString tmpStr;
			m_wndThisStore.GetWindowText(tmpStr);

			tmpStr.Append(_T(" ("));
			tmpStr.Append(s.StoreName);
			tmpStr.Append(_T(")"));

			m_wndThisStore.SetWindowText(tmpStr);
		}
	}

	// Set radio buttons
	m_wndAllStores.SetCheck(!InStore);
	m_wndThisStore.SetCheck(InStore);
	m_wndThisStore.EnableWindow(InStore);

	m_wndList.SetView(LV_VIEW_TILE);
	m_wndList.SetMenus(IDM_CONDITIONLIST);

	// Filter
	if (p_Filter)
	{
		m_wndSearchterm.SetWindowText(p_Filter->Searchterm);

		LFFilterCondition* c = p_Filter->ConditionList;
		while (c)
		{
			m_Conditions.AddItem(*c);
			m_wndList.InsertItem(c);
			c = c->Next;
		}
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFEditFilterDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnEditCondition();
}

void LFEditFilterDlg::OnSave()
{
	LFSaveFilterDlg dlg(this, m_StoreID, TRUE);
	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor csr;

		UINT Result = LFSaveFilter(dlg.m_StoreID, CreateFilter(), dlg.m_FileName, dlg.m_Comments, NULL);
		LFErrorBox(Result, GetSafeHwnd());

		if (Result==LFOk)
			EndDialog(IDOK);
	}
}


void LFEditFilterDlg::OnAddCondition()
{
	LFEditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID);
	if (dlg.DoModal()==IDOK)
	{
		m_Conditions.AddItem(dlg.m_Condition);
		m_wndList.InsertItem(&dlg.m_Condition);
	}

	m_wndList.SetFocus();
}

void LFEditFilterDlg::OnEditCondition()
{
	INT Index = m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (Index!=-1)
	{
		LFEditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID, &m_Conditions.m_Items[Index]);
		if (dlg.DoModal()==IDOK)
		{
			m_Conditions.m_Items[Index] = dlg.m_Condition;
			m_wndList.SetItem(Index, &dlg.m_Condition);
		}

		m_wndList.SetFocus();
	}
}

void LFEditFilterDlg::OnDeleteCondition()
{
	INT Index = m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (Index!=-1)
	{
		m_Conditions.m_ItemCount--;
		for (INT a=Index; a<(INT)m_Conditions.m_ItemCount; a++)
			m_Conditions.m_Items[a] = m_Conditions.m_Items[a+1];

		m_wndList.DeleteItem(Index);
		m_wndList.Arrange(LVA_ALIGNTOP);
	}
}

void LFEditFilterDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_CONDITION_EDIT:
	case IDM_CONDITION_DELETE:
		b = (m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED)!=-1);
		break;
	}

	pCmdUI->Enable(b);
}
