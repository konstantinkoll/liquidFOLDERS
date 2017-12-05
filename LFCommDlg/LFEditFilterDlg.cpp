
// LFEditFilterDlg.cpp: Implementierung der Klasse LFEditFilterDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditConditionDlg.h"
#include "LFSaveFilterDlg.h"


// CConditionList
//

CConditionList::CConditionList()
	: CExplorerList()
{
	for (UINT a=0; a<LFFilterCompareCount; a++)
		ENSURE(m_Compare[a].LoadString(IDS_COMPARE_FIRST+a));
}

void CConditionList::ConditionToItem(LFFilterCondition* pFilterCondition, LVITEM& lvi)
{
	const UINT Attr = pFilterCondition->VData.Attr;

	ZeroMemory(&lvi, sizeof(lvi));

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS;
	lvi.cColumns = 1;
	lvi.puColumns = &lvi.cColumns;
	lvi.pszText = (LPWSTR)LFGetApp()->GetAttributeName(Attr);
	lvi.iImage = LFGetApp()->GetAttributeIcon(Attr)-1;
}

void CConditionList::FinishItem(INT nItem, LFFilterCondition* pFilterCondition)
{
	ASSERT(pFilterCondition->Compare<LFFilterCompareCount);

	WCHAR tmpStr[512];
	wcscpy_s(tmpStr, 512, m_Compare[pFilterCondition->Compare]);

	if (pFilterCondition->Compare)
	{
		wcscat_s(tmpStr, 512, L" ");
		LFVariantDataToString(pFilterCondition->VData, &tmpStr[wcslen(tmpStr)], 512-wcslen(tmpStr));
	}

	SetItemText(nItem, 1, tmpStr);
}

void CConditionList::InsertItem(LFFilterCondition* pFilterCondition)
{
	LVITEM lvi;
	ConditionToItem(pFilterCondition, lvi);
	lvi.iItem = GetItemCount();

	FinishItem(CExplorerList::InsertItem(&lvi), pFilterCondition);
}

void CConditionList::SetItem(INT nItem, LFFilterCondition* pFilterCondition)
{
	LVITEM lvi;
	ConditionToItem(pFilterCondition, lvi);
	lvi.iItem = nItem;

	if (CExplorerList::SetItem(&lvi))
		FinishItem(nItem, pFilterCondition);
}


BEGIN_MESSAGE_MAP(CConditionList, CExplorerList)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CConditionList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_CONDITION_EDIT);

		break;

	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_CONDITION_DELETE);

		break;

	default:
		CExplorerList::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


// LFEditFilterDlg
//

LFEditFilterDlg::LFEditFilterDlg(CWnd* pParentWnd, const LPCSTR StoreID, LFFilter* pFilter)
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
	DDX_Control(pDX, IDC_SEARCHTERM, m_wndSearchTerm);
	DDX_Control(pDX, IDC_CONDITIONLIST, m_wndConditionList);
}

LFFilter* LFEditFilterDlg::CreateFilter()
{
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Mode = LFFilterModeSearch;
	pFilter->Options.IsPersistent = TRUE;

	strcpy_s(pFilter->StoreID, LFKeySize, m_wndAllStores.GetCheck() ? "" : m_StoreID);
	m_wndSearchTerm.GetWindowText(pFilter->SearchTerm, 256);

	for (INT a=m_Conditions.m_ItemCount-1; a>=0; a--)
		pFilter->pConditionList = LFAllocFilterCondition(m_Conditions[a].Compare, m_Conditions[a].VData, pFilter->pConditionList);

	return pFilter;
}

BOOL LFEditFilterDlg::InitDialog()
{
	// Store-Namen einsetzen
	BOOL InStore = FALSE;

	if (m_StoreID[0]!='\0')
	{
		LFStoreDescriptor Store;
		if (LFGetStoreSettings(m_StoreID, Store)==LFOk)
		{
			InStore = TRUE;

			CString tmpStr;
			m_wndThisStore.GetWindowText(tmpStr);

			tmpStr.Append(_T(" ("));
			tmpStr.Append(Store.StoreName);
			tmpStr.Append(_T(")"));

			m_wndThisStore.SetWindowText(tmpStr);
		}
	}

	// Set radio buttons
	m_wndAllStores.SetCheck(!InStore);
	m_wndThisStore.SetCheck(InStore);
	m_wndThisStore.EnableWindow(InStore);

	// List
	m_wndConditionList.SetImageList(&LFGetApp()->m_CoreImageListExtraLarge, LVSIL_NORMAL);

	m_wndConditionList.AddColumn(0);
	m_wndConditionList.AddColumn(1);

	m_wndConditionList.SetMenus(IDM_CONDITION, TRUE, IDM_CONDITIONLIST);
	m_wndConditionList.SetView(LV_VIEW_TILE);
	m_wndConditionList.SetItemsPerRow(1, 2);

	// Filter
	if (p_Filter)
	{
		m_wndSearchTerm.SetWindowText(p_Filter->SearchTerm);

		LFFilterCondition* pFilterCondition = p_Filter->pConditionList;
		while (pFilterCondition)
		{
			m_Conditions.AddItem(*pFilterCondition);
			m_wndConditionList.InsertItem(pFilterCondition);

			pFilterCondition = pFilterCondition->pNext;
		}
	}

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFEditFilterDlg, LFDialog)
	ON_BN_CLICKED(IDM_CONDITIONLIST_ADD, OnAddCondition)
	ON_BN_CLICKED(IDOK, OnSave)
	ON_NOTIFY(NM_DBLCLK, IDC_CONDITIONLIST, OnDoubleClick)

	ON_COMMAND(IDM_CONDITION_EDIT, OnEditCondition)
	ON_COMMAND(IDM_CONDITION_DELETE, OnDeleteCondition)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CONDITION_EDIT, IDM_CONDITION_DELETE, OnUpdateCommands)
END_MESSAGE_MAP()

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

		UINT Result = LFSaveFilter(dlg.m_StoreID, CreateFilter(), dlg.m_FileName, dlg.m_Comments);
		LFErrorBox(this, Result);

		if (Result==LFOk)
			OnOK();
	}
}


void LFEditFilterDlg::OnAddCondition()
{
	LFEditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID);
	if (dlg.DoModal()==IDOK)
	{
		m_Conditions.AddItem(dlg.m_Condition);
		m_wndConditionList.InsertItem(&dlg.m_Condition);
	}

	m_wndConditionList.SetFocus();
}

void LFEditFilterDlg::OnEditCondition()
{
	const INT Index = m_wndConditionList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (Index!=-1)
	{
		LFEditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID, &m_Conditions[Index]);
		if (dlg.DoModal()==IDOK)
		{
			m_Conditions[Index] = dlg.m_Condition;
			m_wndConditionList.SetItem(Index, &dlg.m_Condition);
		}

		m_wndConditionList.SetFocus();
	}
}

void LFEditFilterDlg::OnDeleteCondition()
{
	const INT Index = m_wndConditionList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (Index!=-1)
	{
		m_Conditions.m_ItemCount--;
		for (INT a=Index; a<(INT)m_Conditions.m_ItemCount; a++)
			m_Conditions[a] = m_Conditions[a+1];

		m_wndConditionList.DeleteItem(Index);
		m_wndConditionList.Arrange(LVA_ALIGNTOP);
	}
}

void LFEditFilterDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_CONDITION_EDIT:
	case IDM_CONDITION_DELETE:
		bEnable = (m_wndConditionList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED)!=-1);
		break;
	}

	pCmdUI->Enable(bEnable);
}
