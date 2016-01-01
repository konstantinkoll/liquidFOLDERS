
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFItemTemplateDlg
//

LFItemTemplateDlg::LFItemTemplateDlg(LFItemDescriptor* pItem, const CHAR* pStoreID, CWnd* pParentWnd, BOOL AllowChooseStore, LFFilter* pFilter)
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
					Value.Format(_T("Attr%d"), a);

					BYTE* pData = NULL;
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
		LFFilterCondition* pFilterCondition = pFilter->ConditionList;
		while (pFilterCondition)
		{
			if (pFilterCondition->Compare==LFFilterCompareSubfolder)
			{
				UINT Attr = pFilterCondition->AttrData.Attr;
				if ((!LFGetApp()->m_Attributes[Attr].ReadOnly) && (Attr!=LFAttrFileName))
				{
					ASSERT(m_AttributeValues[Attr].Type==pFilterCondition->AttrData.Type);
					m_AttributeValues[Attr] = pFilterCondition->AttrData;
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
	DDX_Control(pDX, IDC_INSPECTOR, m_wndInspectorGrid);

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

void LFItemTemplateDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), ExplorerHeight, nFlags);
	}

	m_wndInspectorGrid.SetWindowPos(NULL, rectLayout.left+12, rectLayout.top+ExplorerHeight, rectLayout.Width()-12, m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
}

BOOL LFItemTemplateDlg::InitDialog()
{
	m_wndHeaderArea.Create(this, IDC_HEADERAREA);

	if (m_AllowChooseStore)
	{
		CHeaderButton* pButton = m_wndHeaderArea.AddButton();
		pButton->SetDlgCtrlID(IDC_CHOOSESTORE);

		CString tmpStr((LPCSTR)IDC_CHOOSESTORE);
		pButton->SetValue(tmpStr, FALSE);
	}

	// Store
	OnStoresChanged(NULL, NULL);

	// Inspector
	m_wndInspectorGrid.SetStore(m_StoreID);
	m_wndInspectorGrid.AddAttributes(m_AttributeValues);
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);

	for (UINT a=0; a<LFAttributeCount; a++)
		m_wndInspectorGrid.UpdatePropertyState(a, FALSE, !LFGetApp()->m_Attributes[a].ReadOnly, (!LFGetApp()->m_Attributes[a].ReadOnly) && (a!=LFAttrFileName));

	AddBottomRightControl(IDC_SKIP);

	return TRUE;
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
		LFClearVariantData(m_AttributeValues[a]);

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
	LFStoreDescriptor Store;
	if (LFGetStoreSettings(m_StoreID, &Store)==LFOk)
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
			tmpStr = LFGetApp()->m_Attributes[LFAttrCreationTime].Name;
			tmpStr += _T(": ");
			tmpStr += Buffer;
		}

		m_wndHeaderArea.SetText(Store.StoreName, tmpStr);

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
