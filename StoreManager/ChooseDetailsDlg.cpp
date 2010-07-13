#include "StdAfx.h"
#include "ChooseDetailsDlg.h"
#include "LFCore.h"


// ChooseDetailsDlg
//

ChooseDetailsDlg::ChooseDetailsDlg(CWnd* pParentWnd, LFViewParameters* _view, int _context)
	: CAttributeListDialog(IDD_CHOOSEDETAILS, pParentWnd)
{
	ASSERT(_view);
	view = _view;
	context = _context;
}

ChooseDetailsDlg::~ChooseDetailsDlg()
{
}


BEGIN_MESSAGE_MAP(ChooseDetailsDlg, CAttributeListDialog)
	ON_COMMAND(IDC_CHECKALL, OnCheckAll)
	ON_COMMAND(IDC_UNCHECKALL, OnUncheckAll)
END_MESSAGE_MAP()

BOOL ChooseDetailsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_CHOOSEDETAILS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, theApp.m_Contexts[context]->Name);
	SetWindowText(caption);

	// Kontrollelemente einstellen
	PopulateListCtrl(IDC_VIEWATTRIBUTES, ALD_Mode_ChooseDetails, context, view);
	ShowAttributes = ((CListCtrl*)GetDlgItem(IDC_VIEWATTRIBUTES));

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void ChooseDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	CAttributeListDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		BOOL present[LFAttributeCount];
		ZeroMemory(present, sizeof(present));

		// Angezeigte Attribute
		for (int a=0; a<ShowAttributes->GetItemCount(); a++)
		{
			UINT attr = (UINT)ShowAttributes->GetItemData(a);
			present[attr] = TRUE;
			if (ShowAttributes->GetCheck(a)!=(view->ColumnWidth[attr]!=0))
				theApp.ToggleAttribute(view, attr);
		}

		// Nicht angezeigte Attribute
		for (int a=0; a<LFAttributeCount; a++)
			if ((!theApp.m_Attributes[a]->AlwaysVisible) && (!present[a]))
				view->ColumnWidth[a] = 0;
	}
}

void ChooseDetailsDlg::OnCheckAll()
{
	for (int a=0; a<ShowAttributes->GetItemCount(); a++)
		ShowAttributes->SetCheck(a);
}

void ChooseDetailsDlg::OnUncheckAll()
{
	for (int a=0; a<ShowAttributes->GetItemCount(); a++)
		ShowAttributes->SetCheck(a, FALSE);
}
