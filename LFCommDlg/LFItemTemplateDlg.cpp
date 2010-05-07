#include "StdAfx.h"
#include "LFItemTemplateDlg.h"
#include "Resource.h"

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFItemTemplateDlg::LFItemTemplateDlg(CWnd* pParentWnd, LFItemDescriptor* pItem)
	: CDialog(IDD_ITEMTEMPLATE, pParentWnd)
{
	m_pItem = pItem;
	p_App = (LFApplication*)AfxGetApp();

	ZeroMemory(pGroups, sizeof(pGroups));
	ZeroMemory(pAttributes, sizeof(pAttributes));

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		AttributeValues[a].Attr = a;
		LFGetNullVariantData(&AttributeValues[a]);
	}
}

LFItemTemplateDlg::~LFItemTemplateDlg()
{
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

	//TODO
}
