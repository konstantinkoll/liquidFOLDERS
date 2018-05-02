
// LFEditFilterDlg.cpp: Implementierung der Klasse LFEditFilterDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFEditConditionDlg.h"
#include "LFSaveFilterDlg.h"


// CConditionList
//

CConditionList::CConditionList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM)
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CConditionList";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CConditionList", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Conditions = NULL;

	for (UINT a=0; a<LFFilterCompareCount; a++)
		ENSURE(m_Compare[a].LoadString(IDS_COMPARE_FIRST+a));
}

void CConditionList::PreSubclassWindow()
{
	CFrontstageItemView::PreSubclassWindow();

	SetItemHeight(max(m_IconSize=LFGetApp()->m_ExtraLargeIconSize, 2*m_DefaultFontHeight)+2*ITEMVIEWPADDING);
}

void CConditionList::SetConditions(const ConditionArray& Conditions)
{
	SetItemCount((p_Conditions=&Conditions)->m_ItemCount, TRUE);
	ValidateAllItems();

	AdjustLayout();
}

BOOL CConditionList::GetContextMenu(CMenu& Menu, INT Index)
{
	Menu.LoadMenu(Index>=0 ? IDM_CONDITION : IDM_CONDITIONLIST);

	return (Index>=0);
}

void CConditionList::AdjustLayout()
{
	AdjustLayoutColumns();
}

void CConditionList::FireSelectedItem() const
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	GetOwner()->SendMessage(WM_COMMAND, IDM_CONDITION_EDIT);
}

void CConditionList::DeleteSelectedItem() const
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	GetOwner()->PostMessage(WM_COMMAND, IDM_CONDITION_DELETE);
}

void CConditionList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const LFFilterCondition* pFilterCondition = &(*p_Conditions)[Index];
	const UINT Attr = pFilterCondition->VData.Attr;

	CRect rect(rectItem);
	rect.DeflateRect(ITEMVIEWPADDING, ITEMVIEWPADDING);

	// Icon
	const INT IconID = LFGetApp()->GetAttributeIcon(Attr);

	if (IconID)
		LFGetApp()->m_CoreImageListExtraLarge.Draw(&dc, IconID-1, CPoint(rect.left, rect.top+(rect.Height()-m_IconSize)/2), ILD_TRANSPARENT);

	// Text
	CRect rectText(rect);
	rectText.left += m_IconSize+ITEMVIEWPADDING;
	rectText.top += (rect.Height()-2*m_DefaultFontHeight)/2;

	dc.DrawText(LFGetApp()->GetAttributeName(Attr), -1, rectText, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
	rectText.top += m_DefaultFontHeight;

	SetDarkTextColor(dc, Index, Themed);


	WCHAR tmpStr[512];
	wcscpy_s(tmpStr, 512, m_Compare[pFilterCondition->Compare]);

	if (pFilterCondition->Compare)
	{
		wcscat_s(tmpStr, 512, L" ");
		LFVariantDataToString(pFilterCondition->VData, &tmpStr[wcslen(tmpStr)], 512-wcslen(tmpStr));
	}

	dc.DrawText(tmpStr, -1, rectText, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
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

LFFilter* LFEditFilterDlg::CreateFilter() const
{
	LFFilter* pFilter = LFAllocFilter();
	pFilter->IsPersistent = TRUE;

	strcpy_s(pFilter->Query.StoreID, LFKeySize, m_wndAllStores.GetCheck() ? "" : m_StoreID);
	m_wndSearchTerm.GetWindowText(pFilter->Query.SearchTerm, 256);

	for (INT a=m_Conditions.m_ItemCount-1; a>=0; a--)
		pFilter->Query.pConditionList = LFAllocFilterCondition(m_Conditions[a].Compare, m_Conditions[a].VData, pFilter->Query.pConditionList);

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

	// Filter
	if (p_Filter)
	{
		m_wndSearchTerm.SetWindowText(p_Filter->Query.SearchTerm);

		LFFilterCondition* pFilterCondition = p_Filter->Query.pConditionList;
		while (pFilterCondition)
		{
			m_Conditions.AddItem(*pFilterCondition);

			pFilterCondition = pFilterCondition->pNext;
		}

		m_wndConditionList.SetConditions(m_Conditions);
	}

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFEditFilterDlg, LFDialog)
	ON_BN_CLICKED(IDM_CONDITIONLIST_ADD, OnAddCondition)
	ON_BN_CLICKED(IDOK, OnSave)

	ON_COMMAND(IDM_CONDITION_EDIT, OnEditCondition)
	ON_COMMAND(IDM_CONDITION_DELETE, OnDeleteCondition)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CONDITION_EDIT, IDM_CONDITION_DELETE, OnUpdateCommands)
END_MESSAGE_MAP()

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

		m_wndConditionList.SetConditions(m_Conditions);
	}

	m_wndConditionList.SetFocus();
}

void LFEditFilterDlg::OnEditCondition()
{
	const INT Index = GetSelectedCondition();
	if (Index!=-1)
	{
		LFEditConditionDlg dlg(this, m_wndAllStores.GetCheck() ? NULL : m_StoreID, &m_Conditions[Index]);
		if (dlg.DoModal()==IDOK)
		{
			m_Conditions[Index] = dlg.m_Condition;

			m_wndConditionList.Invalidate();
		}

		m_wndConditionList.SetFocus();
	}
}

void LFEditFilterDlg::OnDeleteCondition()
{
	const INT Index = GetSelectedCondition();
	if (Index!=-1)
	{
		for (INT a=Index; a<(INT)m_Conditions.m_ItemCount-1; a++)
			m_Conditions[a] = m_Conditions[a+1];

		m_Conditions.m_ItemCount--;

		m_wndConditionList.SetConditions(m_Conditions);
	}
}

void LFEditFilterDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_CONDITION_EDIT:
	case IDM_CONDITION_DELETE:
		bEnable = (GetSelectedCondition()!=-1);
		break;
	}

	pCmdUI->Enable(bEnable);
}
