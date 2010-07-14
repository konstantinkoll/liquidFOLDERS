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

void ChooseDetailsDlg::SwapItems(int FocusItem, int NewPos)
{
	TCHAR text1[MAX_PATH];
	LVITEM i1;
	ZeroMemory(&i1, sizeof(LVITEM));
	i1.pszText = text1;
	i1.cchTextMax = sizeof(text1)/sizeof(TCHAR);
	i1.iItem = FocusItem;
	i1.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	ShowAttributes->GetItem(&i1);

	TCHAR text2[MAX_PATH];
	LVITEM i2;
	ZeroMemory(&i2, sizeof(LVITEM));
	i2.pszText = text2;
	i2.cchTextMax = sizeof(text2)/sizeof(TCHAR);
	i2.iItem = NewPos;
	i2.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	ShowAttributes->GetItem(&i2);

	std::swap(i1.iItem, i2.iItem);

	ShowAttributes->SetItem(&i1);
	ShowAttributes->SetItem(&i2);

	BOOL Check1 = ShowAttributes->GetCheck(FocusItem);
	BOOL Check2 = ShowAttributes->GetCheck(NewPos);
	ShowAttributes->SetCheck(FocusItem, Check2);
	ShowAttributes->SetCheck(NewPos, Check1);

	ShowAttributes->SetItemState(NewPos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}


BEGIN_MESSAGE_MAP(ChooseDetailsDlg, CAttributeListDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWATTRIBUTES, OnSelectionChange)
	ON_COMMAND(IDC_MOVEUP, OnMoveUp)
	ON_COMMAND(IDC_MOVEDOWN, OnMoveDown)
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
	ShowAttributes = ((CListCtrl*)GetDlgItem(IDC_VIEWATTRIBUTES));
	PopulateListCtrl(IDC_VIEWATTRIBUTES, ALD_Mode_ChooseDetails, context, view);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void ChooseDetailsDlg::OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int idx = (int)pNMListView->iItem;

	if ((pNMListView->uNewState & LVIS_SELECTED) && (ShowAttributes))
	{
		GetDlgItem(IDC_MOVEUP)->EnableWindow(idx>0);
		GetDlgItem(IDC_MOVEDOWN)->EnableWindow(idx<ShowAttributes->GetItemCount()-1);
	}

	*pResult = 0;
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

		// Reihenfolge
		view->ColumnOrder[0] = 0;
		UINT cnt = 1;
		for (int a=0; a<ShowAttributes->GetItemCount(); a++)
			if (ShowAttributes->GetCheck(a))
			{
				UINT colID = 0;
				for (int b=0; b<ShowAttributes->GetItemCount(); b++)
					if ((ShowAttributes->GetCheck(b)) && (ShowAttributes->GetItemData(b)<=ShowAttributes->GetItemData(a)))
						colID++;
				view->ColumnOrder[cnt++] = colID;
			}
	}
}

void ChooseDetailsDlg::OnMoveUp()
{
	int idx = ShowAttributes->GetNextItem(-1, LVIS_SELECTED);
	if (idx>0)
		SwapItems(idx, idx-1);
}

void ChooseDetailsDlg::OnMoveDown()
{
	int idx = ShowAttributes->GetNextItem(-1, LVIS_SELECTED);
	if (idx<ShowAttributes->GetItemCount()-1)
		SwapItems(idx, idx+1);
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
