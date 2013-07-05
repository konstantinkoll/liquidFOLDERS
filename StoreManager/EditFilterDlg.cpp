
// EditFilterDlg.cpp: Implementierung der Klasse EditFilterDlg
//

#include "stdafx.h"
#include "EditConditionDlg.h"
#include "EditFilterDlg.h"
#include "SaveFilterDlg.h"


// EditFilterDlg
//

EditFilterDlg::EditFilterDlg(CWnd* pParentWnd, CHAR* StoreID, LFFilter* pFilter)
	: CDialog(IDD_EDITFILTER, pParentWnd)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");
	p_Filter = pFilter;
}

void EditFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_ALLSTORES, m_wndAllStores);
	DDX_Control(pDX, IDC_THISSTORE, m_wndThisStore);
	DDX_Control(pDX, IDC_SEARCHTERM, m_wndSearchterm);
	DDX_Control(pDX, IDC_CONDITIONLIST, m_wndList);
}

LFFilter* EditFilterDlg::CreateFilter()
{
	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeSearch;
	f->Options.IsPersistent = true;
	strcpy_s(f->StoreID, LFKeySize, m_wndAllStores.GetCheck() ? "" : m_StoreID);
	m_wndSearchterm.GetWindowText(f->Searchterm, 256);

	for (INT a=m_Conditions.m_ItemCount-1; a>=0; a--)
	{
		LFFilterCondition* c = LFAllocFilterCondition();
		*c = m_Conditions.m_Items[a];
		c->Next = f->ConditionList;
		f->ConditionList = c;
	}

	return f;
}


BEGIN_MESSAGE_MAP(EditFilterDlg, CDialog)
	ON_BN_CLICKED(IDM_CONDITIONLIST_ADD, OnAddCondition)
	ON_BN_CLICKED(IDOK, OnSave)
	ON_NOTIFY(NM_DBLCLK, IDC_CONDITIONLIST, OnDoubleClick)

	ON_COMMAND(IDM_CONDITION_EDIT, OnEditCondition)
	ON_COMMAND(IDM_CONDITION_DELETE, OnDeleteCondition)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CONDITION_EDIT, IDM_CONDITION_DELETE, OnUpdateCommands)
END_MESSAGE_MAP()

BOOL EditFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_EDITFILTER);
	SetIcon(hIcon, TRUE);		// Gro�es Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

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

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditFilterDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnEditCondition();
}

void EditFilterDlg::OnSave()
{
	SaveFilterDlg dlg(this, m_StoreID, TRUE);
	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor csr;

		UINT res = LFSaveFilter(dlg.m_StoreID, CreateFilter(), dlg.m_FileName, dlg.m_Comments, NULL);
		LFErrorBox(res, GetSafeHwnd());

		if (res==LFOk)
			EndDialog(IDOK);
	}
}


void EditFilterDlg::OnAddCondition()
{
	EditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID);
	if (dlg.DoModal()==IDOK)
	{
		m_Conditions.AddItem(dlg.m_Condition);
		m_wndList.InsertItem(&dlg.m_Condition);
	}

	m_wndList.SetFocus();
}

void EditFilterDlg::OnEditCondition()
{
	INT idx = m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (idx!=-1)
	{
		EditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID, &m_Conditions.m_Items[idx]);
		if (dlg.DoModal()==IDOK)
		{
			m_Conditions.m_Items[idx] = dlg.m_Condition;
			m_wndList.SetItem(idx, &dlg.m_Condition);
		}

		m_wndList.SetFocus();
	}
}

void EditFilterDlg::OnDeleteCondition()
{
	INT idx = m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (idx!=-1)
	{
		m_Conditions.m_ItemCount--;
		for (INT a=idx; a<(INT)m_Conditions.m_ItemCount; a++)
			m_Conditions.m_Items[a] = m_Conditions.m_Items[a+1];

		m_wndList.DeleteItem(idx);
		m_wndList.Arrange(LVA_ALIGNTOP);
	}
}

void EditFilterDlg::OnUpdateCommands(CCmdUI* pCmdUI)
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
