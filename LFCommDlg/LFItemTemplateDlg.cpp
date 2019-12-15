
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFItemTemplateDlg
//

LFItemTemplateDlg::LFItemTemplateDlg(LFItemDescriptor* pItemDescriptor, const STOREID& StoreID, BOOL AllowChooseStore, CWnd* pParentWnd)
	: LFDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	ASSERT(pItemDescriptor);

	p_ItemDescriptor = pItemDescriptor;
	m_StoreID = StoreID;
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
							memcpy(m_AttributeValues[a].Value, pData, pSz);
							m_AttributeValues[a].IsNull = FALSE;
						}

						free(pData);
					}
				}
	}

	if (pParentWnd)
		pParentWnd->BringWindowToTop();
}

void LFItemTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		p_ItemDescriptor->Type = LFTypeFile;

		for (UINT a=0; a<LFAttributeCount; a++)
			if (!m_AttributeValues[a].IsNull)
				LFSetAttributeVariantData(p_ItemDescriptor, m_AttributeValues[a]);

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
	{
		ASSERT(m_wndInspectorGrid.GetMinWidth()<=rectLayout.Width());

		m_wndInspectorGrid.SetWindowPos(NULL, rectLayout.left, rectLayout.top+ExplorerHeight, rectLayout.Width(), m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
	}
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
		m_wndInspectorGrid.SetStore(m_StoreID=dlg.m_StoreID);

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
	UINT Result;
	LFStoreDescriptor StoreDescriptor;
	if ((Result=LFGetStoreSettings(m_StoreID, StoreDescriptor, TRUE))==LFOk)
	{
		WCHAR tmpBuffer[256];
		LFSizeToString(StoreDescriptor.FreeBytesAvailable.QuadPart, tmpBuffer, 256);

		CString tmpStr;
		tmpStr.Format(IDS_FREEBYTESAVAILABLE, tmpBuffer);

		m_wndHeaderArea.SetHeader(StoreDescriptor.StoreName, tmpStr);
	}

	GetDlgItem(IDOK)->EnableWindow(Result==LFOk);
	GetDlgItem(IDC_SKIP)->EnableWindow(Result==LFOk);

	return NULL;
}
