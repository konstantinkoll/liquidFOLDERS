#include "StdAfx.h"
#include "ViewOptionsDlg.h"
#include "LFCore.h"


// DDX
//

void DDX_ComboBox(CDataExchange* pDX_, int nIDC_, UINT& selection_, int offset)
{
	HWND hWndCtrl = pDX_->PrepareCtrl(nIDC_);
	ASSERT(hWndCtrl);

	CComboBox* pCbx = (CComboBox*)CWnd::FromHandle(hWndCtrl);
	ASSERT(pCbx);

	if (!pDX_->m_bSaveAndValidate)
	{
		pCbx->SetCurSel(selection_ - offset);
	}
	else
	{
		selection_ = pCbx->GetCurSel() + offset;
	}
}


// ViewOptionsDlg
//

ViewOptionsDlg::ViewOptionsDlg(CWnd* pParentWnd, UINT _RibbonColor, LFViewParameters* _view, int _context, LFSearchResult* files)
	: CAttributeListDialog(IDD_VIEWOPTIONS, pParentWnd)
{
	ASSERT(_view!=NULL);
	ShowAttributes = NULL;
	ShowCategories = NULL;
	m_pViewIcons = NULL;
	RibbonColor = _RibbonColor;
	view = _view;
	context = _context;
	if (files)
	{
		HasCategories = files->m_HasCategories;
	}
	else
	{
		HasCategories = TRUE;
	}
}

ViewOptionsDlg::~ViewOptionsDlg()
{
	if (m_pViewIcons)
		delete m_pViewIcons;
}


BEGIN_MESSAGE_MAP(ViewOptionsDlg, CAttributeListDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWMODES, OnViewModeChange)
	ON_NOTIFY(NM_DBLCLK, IDC_VIEWMODES, OnDoubleClick)
	ON_COMMAND(IDC_CHECKALL, OnCheckAll)
	ON_COMMAND(IDC_UNCHECKALL, OnUncheckAll)
END_MESSAGE_MAP()

BOOL ViewOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_VIEWOPTIONS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, theApp.m_Contexts[context]->Name);
	SetWindowText(caption);

	// Background-Überschrift
	GetDlgItem(IDC_BACKGROUNDTEXT)->GetWindowText(BackgroundText);

	// View-Liste füllen
	CStringW tmpStr;

	CListCtrl* l = (CListCtrl*)GetDlgItem(IDC_VIEWMODES);

	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	ENSURE(tmpStr.LoadString(IDS_VIEWGROUP1));
	lvg.pszHeader = tmpStr.GetBuffer();
	lvg.cchHeader = (int)wcslen(lvg.pszHeader);
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.state = LVGS_COLLAPSIBLE;
	if (theApp.osInfo.dwMajorVersion>=6)
	{
		lvg.mask |=  LVGF_STATE;
		lvg.stateMask = LVGS_COLLAPSIBLE;
	}
	l->InsertGroup(lvg.iGroupId, &lvg);

	for (int a=1; a<=3; a++)
	{
		lvg.iGroupId = a;
		ENSURE(tmpStr.LoadString(IDS_VIEWGROUP1+a));
		lvg.pszHeader = tmpStr.GetBuffer();
		l->InsertGroup(lvg.iGroupId, &lvg);
	}

	m_pViewIcons = new CImageListTransparent();
	m_pViewIcons->CreateFromResource(IDB_RIBBONVIEW_16, 0, 12);
	l->SetImageList(m_pViewIcons, LVSIL_SMALL);
	l->SetIconSpacing(68, 30);
	l->EnableGroupView(TRUE);

	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_PARAM | LVIF_STATE;
	lvi.iGroupId = 0;

	for (UINT a=LFViewAutomatic; a<LFViewCount; a++)
	{
		if (theApp.m_Contexts[context]->AllowedViews->IsSet(a))
		{
			lvi.lParam = (LPARAM)a;
			lvi.pszText = theApp.GetCommandName(ID_APP_VIEW_AUTOMATIC+a).AllocSysString();
			lvi.iImage = a;
			lvi.state = lvi.stateMask = (a==view->Mode) ? LVIS_SELECTED | LVIS_FOCUSED : 0;
			l->InsertItem(&lvi);
		}

		if ((a==0) || (a==6) || (a==9))
			lvi.iGroupId++;
	}

	// Kontrollelemente einstellen
	PopulateListCtrl(IDC_VIEWATTRIBUTES, ALD_Mode_ShowAttributes, context, view);
	ShowAttributes = ((CListCtrl*)GetDlgItem(IDC_VIEWATTRIBUTES));
	ShowCategories = ((CButton*)GetDlgItem(IDC_GROUPS));

	NM_LISTVIEW nmlv;
	LRESULT res;
	nmlv.lParam = view->Mode;
	nmlv.uNewState = LVIS_SELECTED;
	OnViewModeChange((NMHDR*)&nmlv, &res);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void ViewOptionsDlg::OnViewModeChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int idx = (int)pNMListView->lParam;

	// Attribute
	if ((pNMListView->uNewState & LVIS_SELECTED) && (ShowAttributes))
	{
		BOOL enable = (idx==LFViewAutomatic) || (idx==LFViewDetails) || (idx==LFViewCalendarDay);
		ShowAttributes->EnableWindow(enable);
		CButton* btn = ((CButton*)GetDlgItem(IDC_CHECKALL));
		if (btn)
			btn->EnableWindow(enable);
		btn = ((CButton*)GetDlgItem(IDC_UNCHECKALL));
		if (btn)
			btn->EnableWindow(enable);
		btn = ((CButton*)GetDlgItem(IDC_SHOWREPORTHEADER));
		if (btn)
			btn->EnableWindow(enable);
		btn = ((CButton*)GetDlgItem(IDC_FULLROWSELECT));
		if (btn)
			btn->EnableWindow(enable);
	}

	// Kategorien
	if ((pNMListView->uNewState & LVIS_SELECTED) && (ShowCategories))
		if ((HasCategories) && (idx>=LFViewAutomatic) && (idx<=LFViewPreview) && (idx!=LFViewList))
		{
			ShowCategories->SetCheck(view->ShowCategories);
			ShowCategories->EnableWindow(TRUE);
		}
		else
		{
			ShowCategories->SetCheck(FALSE);
			ShowCategories->EnableWindow(FALSE);
		}

	// Hintergrund
	GetDlgItem(IDC_BACKGROUNDCOMBO)->EnableWindow(idx>LFViewAutomatic);
	((CComboBox*)GetDlgItem(IDC_BACKGROUNDCOMBO))->SetCurSel(theApp.m_Background[idx]);
	
	CString tmpStr;
	tmpStr.Format(BackgroundText, theApp.GetCommandName(ID_APP_VIEW_AUTOMATIC+idx));
	GetDlgItem(IDC_BACKGROUNDTEXT)->SetWindowText(tmpStr);

	*pResult = 0;
}

void ViewOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CAttributeListDialog::DoDataExchange(pDX);
	DDX_ComboBox(pDX, IDC_COLORCOMBO, RibbonColor, 1);
	DDX_Check(pDX, IDC_FULLROWSELECT, view->FullRowSelect);
	DDX_Check(pDX, IDC_GRANNYMODE, view->GrannyMode);
	DDX_Check(pDX, IDC_ALWAYSSAVE, view->AlwaysSave);
	DDX_Check(pDX, IDC_SHOWQUERYTIMES, theApp.m_ShowQueryDuration);
	if ((HasCategories) && (view->Mode>=LFViewAutomatic) && (view->Mode<=LFViewPreview))
		DDX_Check(pDX, IDC_GROUPS, view->ShowCategories);

	CListCtrl* l = (CListCtrl*)GetDlgItem(IDC_VIEWMODES);
	if (pDX->m_bSaveAndValidate)
	{
		int idx = l->GetNextItem(-1, LVIS_SELECTED);
		if (idx!=-1)
			view->Mode = (UINT)l->GetItemData(idx);

		theApp.m_Background[view->Mode] = ((CComboBox*)GetDlgItem(IDC_BACKGROUNDCOMBO))->GetCurSel();

		if ((view->Mode==LFViewAutomatic) || (view->Mode==LFViewDetails) || (view->Mode==LFViewCalendarDay))
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
}

void ViewOptionsDlg::OnCheckAll()
{
	for (int a=0; a<ShowAttributes->GetItemCount(); a++)
		ShowAttributes->SetCheck(a);
}

void ViewOptionsDlg::OnUncheckAll()
{
	for (int a=0; a<ShowAttributes->GetItemCount(); a++)
		ShowAttributes->SetCheck(a, FALSE);
}

void ViewOptionsDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
