#include "StdAfx.h"
#include "SortOptionsDlg.h"
#include "LFCore.h"


// SortOptionsDlg
//

SortOptionsDlg::SortOptionsDlg(CWnd* pParent, LFViewParameters* _view, int _context, BOOL _IsClipboard)
	: CAttributeListDialog(IDD_SORTOPTIONS, pParent)
{
	ASSERT(_view!=NULL);
	view = _view;
	context = _context;
	IsClipboard = _IsClipboard;
}

SortOptionsDlg::~SortOptionsDlg()
{
}


BEGIN_MESSAGE_MAP(SortOptionsDlg, CAttributeListDialog)
	ON_BN_CLICKED(IDC_AUTODIRS, SetAttrGroupBox)
	ON_NOTIFY(NM_DBLCLK, IDC_SORTATTRIBUTE, OnDoubleClick)
END_MESSAGE_MAP()

BOOL SortOptionsDlg::OnInitDialog()
{
	// Ggf. automatische Verzeichnisse ein- oder ausschalten
	view->AutoDirs |= (view->Mode>LFViewTiles);
	view->AutoDirs &= theApp.m_Contexts[context]->AllowGroups;

	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_SORTOPTIONS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, theApp.m_Contexts[context]->Name);
	SetWindowText(caption);

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_SORTATTRIBUTE, ALD_Mode_SortAttribute, context, view);

	// Ggf. Elemente deaktivieren
	if ((!theApp.m_Contexts[context]->AllowGroups) || (view->Mode>LFViewPreview))
	{
		GetDlgItem(IDC_AUTODIRS)->EnableWindow(FALSE);

		CString hint;
		hint.LoadString(view->AutoDirs ? IDS_SUBFOLDERS_MANDATORY : IDS_SUBFOLDERS_NOTAVAIL);
		GetDlgItem(IDC_SUBFOLDERHINT)->SetWindowText(hint);
	}
	if (view->Mode>LFViewPreview)
	{
		GetDlgItem(IDC_ASCENDING)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESCENDING)->EnableWindow(FALSE);
	}

	// Gruppenbox
	SetAttrGroupBox();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void SortOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CAttributeListDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_ASCENDING, view->Descending);
	DDX_Check(pDX, IDC_AUTODIRS, view->AutoDirs);

	if (pDX->m_bSaveAndValidate)
	{
		// Sortier-Attribut
		HWND hWndCtrl = pDX->PrepareCtrl(IDC_SORTATTRIBUTE);
		ASSERT(hWndCtrl);

		CListCtrl* pList = (CListCtrl*)CWnd::FromHandle(hWndCtrl);
		ASSERT(pList);

		view->SortBy = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}

void SortOptionsDlg::SetAttrGroupBox()
{
	CString caption;
	caption.LoadString(IsDlgButtonChecked(IDC_AUTODIRS) ? IDS_GROUPBOX_GROUP : IDS_GROUPBOX_SORT);
	GetDlgItem(IDC_ATTRBOX)->SetWindowText(caption);
}

void SortOptionsDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
