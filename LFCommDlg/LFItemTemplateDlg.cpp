
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "StdAfx.h"
#include "LFItemTemplateDlg.h"
#include "LFChooseStoreDlg.h"
#include "Resource.h"


// LFItemTemplateDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFItemTemplateDlg::LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, CHAR* StoreID, BOOL AllowChooseStore, LFFilter* pFilter)
	: CDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	m_pItem = pItem;
	p_App = (LFApplication*)AfxGetApp();
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
	DDX_Control(pDX, IDC_DESTINATION, m_wndStorePanel);
	DDX_Control(pDX, IDC_INSPECTOR, m_wndInspectorGrid);
	DDX_Check(pDX, IDC_ALPHABETICALLY, m_SortAlphabetic);

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


BEGIN_MESSAGE_MAP(LFItemTemplateDlg, CDialog)
	ON_BN_CLICKED(IDC_CHOOSESTORE, OnChooseStore)
	ON_BN_CLICKED(IDC_ALPHABETICALLY, OnSortAlphabetic)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)
	ON_REGISTERED_MESSAGE(((LFApplication*)AfxGetApp())->p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(((LFApplication*)AfxGetApp())->p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(((LFApplication*)AfxGetApp())->p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

BOOL LFItemTemplateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_ITEMTEMPLATE));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Größe
	CRect rect;
	GetWindowRect(&rect);

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	CRect rectInspector;
	m_wndInspectorGrid.GetWindowRect(&rectInspector);

	INT Grow = min(550-rectInspector.Height(), rectScreen.Height()/2-rectInspector.Height());
	if (Grow>0)
	{
#define Grow(pWnd) { pWnd->GetWindowRect(&rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height()+Grow, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE); }
#define Move(pWnd) { pWnd->GetWindowRect(&rect); ScreenToClient(rect); pWnd->SetWindowPos(NULL, rect.left, rect.top+Grow, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE); }

		Grow(this);
		Grow(GetDlgItem(IDC_GROUPBOX1));
		Grow(GetDlgItem(IDC_INSPECTOR));
		Move(GetDlgItem(IDC_ALPHABETICALLY));
		Move(GetDlgItem(IDC_RESET));
		Move(GetDlgItem(IDOK));
		Move(GetDlgItem(IDC_SKIP));
		Move(GetDlgItem(IDCANCEL));
	}

	// Store
	GetDlgItem(IDC_CHOOSESTORE)->EnableWindow(m_AllowChooseStore);
	OnStoresChanged(NULL, NULL);

	// Inspector
	m_wndInspectorGrid.GetWindowRect(&rectInspector);
	ScreenToClient(rectInspector);
	m_FrameCtrl.Create(this, rectInspector);

	rectInspector.DeflateRect(2, 2);
	m_wndInspectorGrid.SetWindowPos(NULL, rectInspector.left, rectInspector.top, rectInspector.Width(), rectInspector.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	m_wndInspectorGrid.SetStore(m_StoreID);
	m_wndInspectorGrid.AddAttributes(m_AttributeValues);

	for (UINT a=0; a<LFAttributeCount; a++)
		m_wndInspectorGrid.UpdatePropertyState(a, FALSE, !p_App->m_Attributes[a].ReadOnly, (!p_App->m_Attributes[a].ReadOnly) && (a!=LFAttrFileName));

	OnSortAlphabetic();

	return TRUE;
}

void LFItemTemplateDlg::OnChooseStore()
{
	LFChooseStoreDlg dlg(this, LFCSD_Mounted);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(m_StoreID, LFKeySize, dlg.m_StoreID);
		m_wndInspectorGrid.SetStore(m_StoreID);
		OnStoresChanged(NULL, NULL);
	}
}

void LFItemTemplateDlg::OnSortAlphabetic()
{
	m_SortAlphabetic = ((CButton*)GetDlgItem(IDC_ALPHABETICALLY))->GetCheck();
	m_wndInspectorGrid.SetAlphabeticMode(m_SortAlphabetic);
}

void LFItemTemplateDlg::OnReset()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		LFGetNullVariantData(&m_AttributeValues[a]);

	m_wndInspectorGrid.Invalidate();
	m_wndInspectorGrid.SetFocus();
}

void LFItemTemplateDlg::OnSkip()
{
	EndDialog(IDC_SKIP);
}

LRESULT LFItemTemplateDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStorePanel.SetStore(m_StoreID);
	GetDlgItem(IDOK)->EnableWindow(m_wndStorePanel.IsValidStore());
	GetDlgItem(IDC_SKIP)->EnableWindow(m_wndStorePanel.IsValidStore());

	return NULL;
}
