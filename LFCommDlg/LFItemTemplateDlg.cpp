
// LFItemTemplateDlg.cpp: Implementierung der Klasse LFItemTemplateDlg
//

#include "StdAfx.h"
#include "LFItemTemplateDlg.h"
#include "Resource.h"


// LFItemTemplateDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFItemTemplateDlg::LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem, char* _StoreID)
	: CDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	m_pItem = pItem;
	p_App = (LFApplication*)AfxGetApp();
	strcpy_s(StoreID, LFKeySize, _StoreID);

	ZeroMemory(pGroups, sizeof(pGroups));
	ZeroMemory(pAttributes, sizeof(pAttributes));

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		AttributeValues[a].Attr = a;
		LFGetNullVariantData(&AttributeValues[a]);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.Open(_T("Software\\liquidFOLDERS\\Template")))
	{
		int count = 0;
		if (reg.Read(_T("AttrCount"), count))
			if (count==LFAttributeCount)
				for (unsigned int a=0; a<LFAttributeCount; a++)
				{
					CString value;
					value.Format(_T("Attr%d"), a);

					BYTE* pData = NULL;
					UINT pSz = 0;

					if (reg.Read(value, &pData, &pSz))
					{
						if (pSz==sizeof(AttributeValues[a].Value))
						{
							memcpy_s(AttributeValues[a].Value, sizeof(AttributeValues[a].Value), pData, pSz);
							AttributeValues[a].IsNull = false;
						}

						free(pData);
					}
				}
	}

	CFrameWnd* Frame = pParentWnd->GetParentFrame();
	if (Frame)
	{
		Frame->BringWindowToTop();
	}
	else
	{
		pParentWnd->BringWindowToTop();
	}
}


BEGIN_MESSAGE_MAP(LFItemTemplateDlg, CDialog)
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

	m_Inspector.ModifyStyle(0, WS_BORDER);
	m_Inspector.SetGroupNameFullWidth(TRUE, FALSE);
	m_Inspector.EnableHeaderCtrl(FALSE);
	m_Inspector.MarkModifiedProperties(TRUE);
	m_Inspector.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)), FALSE);
	OnSortAlphabetically();

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		pGroups[a] = new CMFCPropertyGridProperty(p_App->m_AttrCategories[a]);

	for (UINT a=0; a<LFAttributeCount; a++)
		if ((!p_App->m_Attributes[a]->ReadOnly) && (a!=LFAttrFileName))
		{
			switch (AttributeValues[a].Type)
			{
			case LFTypeUnicodeArray:
				pAttributes[a] = new CAttributePropertyTags(&AttributeValues[a], StoreID);
				break;
			case LFTypeAnsiString:
				pAttributes[a] = (a==LFAttrLocationIATA) ? new CAttributePropertyIATA(&AttributeValues[a], (CAttributeProperty**)&pAttributes[LFAttrLocationName], (CAttributeProperty**)&pAttributes[LFAttrLocationGPS]) : new CAttributeProperty(&AttributeValues[a]);
				break;
			case LFTypeRating:
				pAttributes[a] = new CAttributePropertyRating(&AttributeValues[a]);
				break;
			case LFTypeGeoCoordinates:
				pAttributes[a] = new CAttributePropertyGPS(&AttributeValues[a]);
				break;
			case LFTypeTime:
				pAttributes[a] = new CAttributePropertyTime(&AttributeValues[a]);
				break;
			default:
				pAttributes[a] = new CAttributeProperty(&AttributeValues[a]);
			}

			pGroups[p_App->m_Attributes[a]->Category]->AddSubItem(pAttributes[a]);
		}

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		if (pGroups[a]->GetSubItemsCount())
		{
			m_Inspector.AddProperty(pGroups[a]);
		}
		else
		{
			delete pGroups[a];
			pGroups[a] = NULL;
		}

	return TRUE;
}

void LFItemTemplateDlg::OnSortAlphabetically()
{
	m_Inspector.SetAlphabeticMode(((CButton*)GetDlgItem(IDC_ALPHABETICALLY))->GetCheck());
}

void LFItemTemplateDlg::OnReset()
{
	for (UINT a=0; a<LFAttributeCount; a++)
	{
		LFGetNullVariantData(&AttributeValues[a]);

		if (pAttributes[a])
			pAttributes[a]->SetValue(_T(""), FALSE);
	}

	m_Inspector.SetFocus();
}

void LFItemTemplateDlg::OnSkip()
{
	EndDialog(IDC_SKIP);
}

void LFItemTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INSPECTOR, m_Inspector);

	if (pDX->m_bSaveAndValidate)
	{
		m_pItem->Type = LFTypeFile;

		for (unsigned int a=0; a<LFAttributeCount; a++)
			if ((pAttributes[a]) && (!AttributeValues[a].IsNull))
				LFSetAttributeVariantData(m_pItem, &AttributeValues[a]);

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);

		if (reg.CreateKey(_T("Software\\liquidFOLDERS\\Template")))
		{
			reg.Write(_T("AttrCount"), LFAttributeCount);

			for (unsigned int a=0; a<LFAttributeCount; a++)
			{
				CString value;
				value.Format(_T("Attr%d"), a);

				if ((pAttributes[a]) && (!AttributeValues[a].IsNull))
				{
					reg.Write(value, (BYTE*)&AttributeValues[a].Value, sizeof(AttributeValues[a].Value));
				}
				else
				{
					reg.DeleteValue(value);
				}
			}
		}
	}
}
