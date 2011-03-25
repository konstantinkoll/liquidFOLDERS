
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "StdAfx.h"
#include "LFItemTemplateDlg.h"
#include "Resource.h"


// LFItemTemplateDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFItemTemplateDlg::LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, CHAR* StoreID)
	: CDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	m_pItem = pItem;
	p_App = (LFApplication*)AfxGetApp();
	strcpy_s(m_StoreID, LFKeySize, StoreID);

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		m_AttributeValues[a].Attr = a;
		LFGetNullVariantData(&m_AttributeValues[a]);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.Open(_T("Software\\liquidFOLDERS\\Template")))
	{
		INT count = 0;
		if (reg.Read(_T("AttrCount"), count))
			if (count==LFAttributeCount)
				for (UINT a=0; a<LFAttributeCount; a++)
				{
					CString value;
					value.Format(_T("Attr%d"), a);

					BYTE* pData = NULL;
					UINT pSz = 0;

					if (reg.Read(value, &pData, &pSz))
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
			reg.Write(_T("AttrCount"), LFAttributeCount);

			for (UINT a=0; a<LFAttributeCount; a++)
			{
				CString value;
				value.Format(_T("Attr%d"), a);

				if (m_AttributeValues[a].IsNull)
				{
					reg.DeleteValue(value);
				}
				else
				{
					reg.Write(value, (BYTE*)&m_AttributeValues[a].Value, sizeof(m_AttributeValues[a].Value));
				}
			}
		}
	}
}


BEGIN_MESSAGE_MAP(LFItemTemplateDlg, CDialog)
	ON_WM_WINDOWPOSCHANGED()
	ON_BN_CLICKED(IDC_ALPHABETICALLY, OnSortAlphabetically)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)
END_MESSAGE_MAP()


BOOL LFItemTemplateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_ITEMTEMPLATE));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	CRect rect;
	m_wndInspectorGrid.GetWindowRect(&rect);
	ScreenToClient(rect);
	m_FrameCtrl.Create(this, rect);

	rect.DeflateRect(2, 2);
	m_wndInspectorGrid.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	m_wndInspectorGrid.AddAttributes(m_AttributeValues);

	for (UINT a=0; a<LFAttributeCount; a++)
		m_wndInspectorGrid.UpdatePropertyState(a, FALSE, !p_App->m_Attributes[a]->ReadOnly, (!p_App->m_Attributes[a]->ReadOnly) && (a!=LFAttrFileName));

	OnSortAlphabetically();

	return TRUE;
}

void LFItemTemplateDlg::OnSortAlphabetically()
{
	m_wndInspectorGrid.SetAlphabeticMode(((CButton*)GetDlgItem(IDC_ALPHABETICALLY))->GetCheck());
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
