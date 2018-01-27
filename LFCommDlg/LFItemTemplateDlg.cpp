
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFItemTemplateDlg
//

LFItemTemplateDlg::LFItemTemplateDlg(LFItemDescriptor* pItem, const LPCSTR pStoreID, CWnd* pParentWnd, BOOL AllowChooseStore, LFFilter* pFilter)
	: LFDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	ASSERT(pStoreID);

	m_pItem = pItem;
	strcpy_s(m_StoreID, LFKeySize, pStoreID);
	m_AllowChooseStore = AllowChooseStore;
	m_SortAlphabetic = FALSE;

	for (UINT a=0; a<LFAttributeCount; a++)
		LFInitVariantData(m_AttributeValues[a], a);

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
					Value.Format(_T("Attr%u"), a);

					LPBYTE pData = NULL;
					UINT pSz = 0;

					if (reg.Read(Value, &pData, &pSz))
					{
						if (pSz==sizeof(m_AttributeValues[a].Value))
						{
							memcpy_s(m_AttributeValues[a].Value, sizeof(m_AttributeValues[a].Value), pData, pSz);
							m_AttributeValues[a].IsNull = FALSE;
						}

						free(pData);
					}
				}
	}

	if (pFilter)
	{
		LFFilterCondition* pFilterCondition = pFilter->Query.pConditionList;
		while (pFilterCondition)
		{
			if (pFilterCondition->Compare==LFFilterCompareSubfolder)
			{
				const UINT Attr = pFilterCondition->VData.Attr;

				if (LFGetApp()->IsAttributeEditable(Attr) && (Attr!=LFAttrFileName))
				{
					ASSERT(m_AttributeValues[Attr].Type==pFilterCondition->VData.Type);
					m_AttributeValues[Attr] = pFilterCondition->VData;
				}
			}

			pFilterCondition = pFilterCondition->pNext;
		}
	}

	if (pParentWnd)
		pParentWnd->BringWindowToTop();
}

void LFItemTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		m_pItem->Type = LFTypeFile;

		for (UINT a=0; a<LFAttributeCount; a++)
			if (!m_AttributeValues[a].IsNull)
				LFSetAttributeVariantData(m_pItem, m_AttributeValues[a]);

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);

		if (reg.CreateKey(_T("Software\\liquidFOLDERS\\Template")))
		{
			reg.Write(_T("SortAlphabetic"), m_SortAlphabetic);
			reg.Write(_T("AttrCount"), LFAttributeCount);

			for (UINT a=0; a<LFAttributeCount; a++)
			{
				CString Value;
				Value.Format(_T("Attr%u"), a);

				if (m_AttributeValues[a].IsNull)
				{
					reg.DeleteValue(Value);
				}
				else
				{
					reg.Write(Value, (LPBYTE)&m_AttributeValues[a].Value, sizeof(m_AttributeValues[a].Value));
				}
			}
		}
	}
}

void LFItemTemplateDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), ExplorerHeight, nFlags);
	}

	if (IsWindow(m_wndInspectorGrid))
		m_wndInspectorGrid.SetWindowPos(NULL, rectLayout.left+BACKSTAGEBORDER, rectLayout.top+ExplorerHeight, rectLayout.Width()-BACKSTAGEBORDER, m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
}

BOOL LFItemTemplateDlg::InitDialog()
{
	m_wndHeaderArea.Create(this, IDC_HEADERAREA);

	if (m_AllowChooseStore)
		m_wndHeaderArea.AddButton(IDC_CHOOSESTORE);

	// Store
	OnStoresChanged(NULL, NULL);

	// Inspector
	m_wndInspectorGrid.Create(this, IDC_INSPECTOR, IDM_ITEMTEMPLATE);

	m_wndInspectorGrid.SetStore(m_StoreID);
	m_wndInspectorGrid.AddAttributeProperties(m_AttributeValues);
	m_wndInspectorGrid.SetFocus();

	// Sort inspector items
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);

	// "Skip" button
	AddBottomRightControl(IDC_SKIP);

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFItemTemplateDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_CHOOSESTORE, OnChooseStore)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)

	ON_COMMAND(IDM_ITEMTEMPLATE_TOGGLESORT, OnToggleSort)
	ON_COMMAND(IDM_ITEMTEMPLATE_RESET, OnReset)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ITEMTEMPLATE_TOGGLESORT, IDM_ITEMTEMPLATE_RESET, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI(IDC_CHOOSESTORE, OnUpdateCommands)

	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

void LFItemTemplateDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	if (IsWindowVisible())
	{
		CRect rect;
		GetWindowRect(rect);

		lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = rect.Width();
	}

	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
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

void LFItemTemplateDlg::OnSkip()
{
	EndDialog(IDC_SKIP);
}


void LFItemTemplateDlg::OnToggleSort()
{
	m_SortAlphabetic = !m_SortAlphabetic;
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);
}

void LFItemTemplateDlg::OnReset()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		LFClearVariantData(m_AttributeValues[a]);

	m_wndInspectorGrid.Invalidate();
}

void LFItemTemplateDlg::OnUpdateCommands(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_nID==IDM_ITEMTEMPLATE_TOGGLESORT)
		pCmdUI->SetCheck(m_SortAlphabetic);

	pCmdUI->Enable(TRUE);
}


LRESULT LFItemTemplateDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	LFStoreDescriptor Store;
	if (LFGetStoreSettings(m_StoreID, Store)==LFOk)
	{
		CString tmpStr;
		if (Store.Comments[0]!=L'\0')
		{
			tmpStr = Store.Comments;
		}
		else
		{
			WCHAR Buffer[256];
			LFTimeToString(Store.CreationTime, Buffer, 256);

			LFTooltip::AppendAttribute(tmpStr, LFGetApp()->GetAttributeName(LFAttrCreationTime, LFContextStores), Buffer);
		}

		m_wndHeaderArea.SetHeader(Store.StoreName, tmpStr);

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
