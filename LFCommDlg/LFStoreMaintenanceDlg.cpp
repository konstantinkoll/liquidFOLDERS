
// LFStoreMaintenanceDlg.cpp: Implementierung der Klasse LFStoreMaintenanceDlg
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include "Resource.h"


// LFStoreMaintenanceDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFStoreMaintenanceDlg::LFStoreMaintenanceDlg(LFMaintenanceList* ml, CWnd* pParentWnd)
	: CDialog(IDD_STOREMAINTENANCE, pParentWnd)
{
	ASSERT(ml);

	for (UINT a=0; a<ml->m_ItemCount; a++)
		m_Lists[ml->m_Items[a].Result==LFOk ? 0 : 1].AddItem(&ml->m_Items[a]);

	m_Page = 0;
}

void LFStoreMaintenanceDlg::SetPage(INT page)
{
	ASSERT((page==0) || (page==1));

	m_Page = page;

	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_STORELIST);
	li->SetRedraw(FALSE);
	li->SetItemCount(0);

	li->SetItemCount(m_Lists[m_Page].m_ItemCount);
	li->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	li->SetItemState(0, LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	li->SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	li->EnsureVisible(0, FALSE);

	li->SetRedraw(TRUE);
	li->Invalidate();

	if (!m_Lists[m_Page].m_ItemCount)
		GetDlgItem(IDC_STATUS)->SetWindowText(_T(""));
}


BEGIN_MESSAGE_MAP(LFStoreMaintenanceDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABS, OnTabChanged)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_STORELIST, OnGetDispInfo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STORELIST, OnItemChanged)
END_MESSAGE_MAP()

BOOL LFStoreMaintenanceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STOREMAINTENANCE));
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Icons
	m_Icons.Create(16, 16, ILC_COLOR32, 1, 1);

	hIcon = (HICON)LoadImage(LFCommDlgDLL.hResource, IDI_EXCLAMATION, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_Icons.Add(hIcon);
	DestroyIcon(hIcon);

	// Tabs
	CTabCtrl* tabs = (CTabCtrl*)GetDlgItem(IDC_TABS);
	tabs->SetImageList(&m_Icons);

	for (UINT a=0; a<(m_Lists[1].m_ItemCount ? (UINT)2 : (UINT)1); a++)
	{
		CString mask;
		ENSURE(mask.LoadString(IDS_MAINTENANCETAB0+a));

		CString tmpStr;
		tmpStr.Format(mask, m_Lists[a].m_ItemCount);

		tabs->InsertItem(a, tmpStr, (INT)a-1);
	}

	// Liste
	LFApplication* pApp = LFGetApp();
	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_STORELIST);
	li->SetExtendedStyle(li->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	li->SetImageList(&pApp->m_CoreImageListSmall, LVSIL_SMALL);
	li->SetFont(&pApp->m_DefaultFont, FALSE);
	li->InsertColumn(0, pApp->m_Attributes[LFAttrFileName]->Name);
	li->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

	// Seite
	tabs->SetCurSel(m_Lists[1].m_ItemCount ? 1 : 0);
	SetPage(tabs->GetCurSel());

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

HBRUSH LFStoreMaintenanceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_STATIC) && IsCtrlThemed())
	{
		pDC->SetBkColor(0xFFFFFF);

		hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	return hbr;
}

void LFStoreMaintenanceDlg::OnTabChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTabCtrl* tabs = (CTabCtrl*)GetDlgItem(IDC_TABS);
	SetPage(tabs->GetCurSel());
}

void LFStoreMaintenanceDlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	LFML_Item* pStore = m_Lists[m_Page].m_Items[pItem->iItem];

	if (pItem->mask & LVIF_TEXT)
		pItem->pszText = pStore->Name;

	if (pItem->mask & LVIF_IMAGE)
		pItem->iImage = pStore->Icon-1;
}

void LFStoreMaintenanceDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		LFML_Item* pStore = m_Lists[m_Page].m_Items[pNMListView->iItem];

		if (m_Page==0)
		{
			GetDlgItem(IDC_STATUS)->SetWindowText(_T(""));
		}
		else
		{
			WCHAR* tmpStr = LFGetErrorText(pStore->Result);
			GetDlgItem(IDC_STATUS)->SetWindowText(tmpStr);
			free(tmpStr);
		}
	}
}
