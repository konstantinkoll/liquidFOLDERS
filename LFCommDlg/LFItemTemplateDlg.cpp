
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include "Resource.h"


// LFItemTemplateDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFItemTemplateDlg::LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, CHAR* StoreID, BOOL AllowChooseStore, LFFilter* pFilter)
	: LFDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	m_pItem = pItem;
	p_App = LFGetApp();
	strcpy_s(m_StoreID, LFKeySize, StoreID);
	m_AllowChooseStore = AllowChooseStore;
	m_SortAlphabetic = FALSE;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		m_AttributeValues[a].Attr = a;
		LFGetNullVariantData(&m_AttributeValues[a]);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.Open(_T("Software\\liquidFOLDERS\\Template")))
	{
		reg.Read(_T("SortAlphabetic"), m_SortAlphabetic);

		INT Count = 0;
		if (reg.Read(_T("AttrCount"), Count))
			if (Count==LFAttributeCount)
				for (UINT a=0; a<LFAttributeCount; a++)
				{
					CString Value;
					Value.Format(_T("Attr%d"), a);

					BYTE* pData = NULL;
					UINT pSz = 0;

					if (reg.Read(Value, &pData, &pSz))
					{
						if (pSz==sizeof(m_AttributeValues[a].Value))
						{
							memcpy_s(m_AttributeValues[a].Value, sizeof(m_AttributeValues[a].Value), pData, pSz);
							m_AttributeValues[a].IsNull = false;
						}

						free(pData);
					}
				}
	}

	if (pFilter)
	{
		LFFilterCondition* pCondition = pFilter->ConditionList;
		while (pCondition)
		{
			if (pCondition->Compare==LFFilterCompareSubfolder)
			{
				UINT Attr = pCondition->AttrData.Attr;
				if ((!p_App->m_Attributes[Attr].ReadOnly) && (Attr!=LFAttrFileName))
				{
					ASSERT(m_AttributeValues[Attr].Type==pCondition->AttrData.Type);
					m_AttributeValues[Attr] = pCondition->AttrData;
				}
			}

			pCondition = pCondition->Next;
		}
	}

	if (pParentWnd)
		pParentWnd->BringWindowToTop();
}

void LFItemTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_INSPECTOR, m_wndInspectorGrid);

	if (pDX->m_bSaveAndValidate)
	{
		m_pItem->Type = LFTypeFile;

		for (UINT a=0; a<LFAttributeCount; a++)
			if (!m_AttributeValues[a].IsNull)
				LFSetAttributeVariantData(m_pItem, &m_AttributeValues[a]);

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);

		if (reg.CreateKey(_T("Software\\liquidFOLDERS\\Template")))
		{
			reg.Write(_T("SortAlphabetic"), m_SortAlphabetic);
			reg.Write(_T("AttrCount"), LFAttributeCount);

			for (UINT a=0; a<LFAttributeCount; a++)
			{
				CString Value;
				Value.Format(_T("Attr%d"), a);

				if (m_AttributeValues[a].IsNull)
				{
					reg.DeleteValue(Value);
				}
				else
				{
					reg.Write(Value, (BYTE*)&m_AttributeValues[a].Value, sizeof(m_AttributeValues[a].Value));
				}
			}
		}
	}
}

void LFItemTemplateDlg::AdjustLayout()
{
	if (!IsWindow(m_wndInspectorGrid))
		return;

	CRect rect;
	GetLayoutRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	m_wndInspectorGrid.SetWindowPos(NULL, rect.left+13, rect.top+ExplorerHeight, rect.Width()-13, rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(LFItemTemplateDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_CHOOSESTORE, OnChooseStore)
	ON_COMMAND(IDM_ITEMTEMPLATE_TOGGLESORT, OnToggleSort)
	ON_COMMAND(IDM_ITEMTEMPLATE_RESET, OnReset)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ITEMTEMPLATE_TOGGLESORT, IDM_ITEMTEMPLATE_RESET, OnUpdateCommands)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

BOOL LFItemTemplateDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	m_wndHeaderArea.Create(this, IDC_HEADERAREA);

	if (m_AllowChooseStore)
	{
		CHeaderButton* pButton = m_wndHeaderArea.AddButton();
		pButton->SetDlgCtrlID(IDC_CHOOSESTORE);

		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDC_CHOOSESTORE));
		pButton->SetValue(tmpStr, FALSE);
	}

	// Store
	OnStoresChanged(NULL, NULL);

	// Inspector
	m_wndInspectorGrid.SetStore(m_StoreID);
	m_wndInspectorGrid.AddAttributes(m_AttributeValues);
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);

	for (UINT a=0; a<LFAttributeCount; a++)
		m_wndInspectorGrid.UpdatePropertyState(a, FALSE, !p_App->m_Attributes[a].ReadOnly, (!p_App->m_Attributes[a].ReadOnly) && (a!=LFAttrFileName));

	AdjustLayout();
	AddBottomRightControl(IDC_SKIP);

	return TRUE;
}

void LFItemTemplateDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	CRect rect;
	GetWindowRect(rect);
	if (rect.Width())
		lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = rect.Width();

	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

void LFItemTemplateDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd!=&m_wndInspectorGrid)
		return;

	if ((point.x<0) || (point.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		point.x = (rect.left+rect.right)/2;
		point.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&point);
	}

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_ITEMTEMPLATE));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
}

void LFItemTemplateDlg::OnChooseStore()
{
	LFChooseStoreDlg dlg(this);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(m_StoreID, LFKeySize, dlg.m_StoreID);
		m_wndInspectorGrid.SetStore(m_StoreID);

		OnStoresChanged(NULL, NULL);
	}
}

void LFItemTemplateDlg::OnToggleSort()
{
	m_SortAlphabetic = !m_SortAlphabetic;
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);
}

void LFItemTemplateDlg::OnReset()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		LFGetNullVariantData(&m_AttributeValues[a]);

	m_wndInspectorGrid.Invalidate();
}

void LFItemTemplateDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_nID==IDM_ITEMTEMPLATE_TOGGLESORT)
		pCmdUI->SetCheck(m_SortAlphabetic);

	pCmdUI->Enable(TRUE);
}


void LFItemTemplateDlg::OnSkip()
{
	EndDialog(IDC_SKIP);
}

LRESULT LFItemTemplateDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	LFStoreDescriptor store;
	if (LFGetStoreSettings(m_StoreID, &store)==LFOk)
	{
		CString tmpStr;
		if (store.StoreComment[0]!=L'\0')
		{
			tmpStr = store.StoreComment;
		}
		else
		{
			WCHAR Buffer[256];
			LFTimeToString(store.CreationTime, Buffer, 256);
			tmpStr = p_App->m_Attributes[LFAttrCreationTime].Name;
			tmpStr += _T(": ");
			tmpStr += Buffer;
		}

		m_wndHeaderArea.SetText(store.StoreName, tmpStr);

		GetDlgItem(IDOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_SKIP)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_SKIP)->EnableWindow(FALSE);
	}

	return NULL;
}
