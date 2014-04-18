
// ReportDlg.cpp: Implementierung der Klasse ReportDlg
//

#include "stdafx.h"
#include "ReportDlg.h"
#include "LFCommDlg.h"
#include "resource.h"


// ReportDlg
//

ReportDlg::ReportDlg(CWnd* pParentWnd, CReportList* Successful, CReportList* WithErrors)
	: CDialog(IDD_REPORT, pParentWnd)
{
	ASSERT(Successful);
	ASSERT(WithErrors);

	m_Lists[0] = Successful;
	m_Lists[1] = WithErrors;
	m_Page = 0;
	m_UncheckMigrated = TRUE;
}

void ReportDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX, IDC_UNCHECKMIGRATED, m_UncheckMigrated);
}

void ReportDlg::SetPage(INT page)
{
	ASSERT((page==0) || (page==1));

	m_Page = page;

	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_FOLDERLIST);
	li->SetRedraw(FALSE);
	li->SetItemCount(0);

	li->SetItemCount(m_Lists[m_Page]->m_ItemCount);
	li->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	li->SetItemState(0, LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	li->SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	li->EnsureVisible(0, FALSE);

	li->SetRedraw(TRUE);
	li->Invalidate();

	if (!m_Lists[m_Page]->m_ItemCount)
	{
		GetDlgItem(IDC_STATUS1)->SetWindowText(_T(""));
		GetDlgItem(IDC_STATUS2)->SetWindowText(_T(""));
	}
}


BEGIN_MESSAGE_MAP(ReportDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABS, OnTabChanged)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_FOLDERLIST, OnGetDispInfo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FOLDERLIST, OnItemChanged)
END_MESSAGE_MAP()

BOOL ReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_REPORT);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Icons
	m_Icons.Create(16, 16, ILC_COLOR32, 1, 1);
	m_Icons.Add((HICON)LoadImage(GetModuleHandle(_T("LFCOMMDLG.DLL")), IDI_EXCLAMATION, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));

	// Tabs
	CTabCtrl* tabs = (CTabCtrl*)GetDlgItem(IDC_TABS);
	tabs->SetImageList(&m_Icons);

	for (UINT a=0; a<(m_Lists[1]->m_ItemCount ? (UINT)2 : (UINT)1); a++)
	{
		CString mask;
		ENSURE(mask.LoadString(IDS_REPORTTAB0+a));

		CString tmpStr;
		tmpStr.Format(mask, m_Lists[a]->m_ItemCount);

		tabs->InsertItem(a, tmpStr, (INT)a-1);
	}

	// Liste
	CListCtrl* li = (CListCtrl*)GetDlgItem(IDC_FOLDERLIST);
	li->SetExtendedStyle(li->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	li->SetImageList(&theApp.m_SystemImageListSmall, LVSIL_SMALL);
	li->SetFont(&theApp.m_DefaultFont, FALSE);
	li->InsertColumn(0, theApp.m_Attributes[LFAttrFileName]->Name);
	li->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

	// Checkbox
	GetDlgItem(IDC_UNCHECKMIGRATED)->EnableWindow(m_Lists[0]->m_ItemCount);

	// Seite
	tabs->SetCurSel(m_Lists[1]->m_ItemCount ? 1 : 0);
	SetPage(tabs->GetCurSel());

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

HBRUSH ReportDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_STATIC) && IsCtrlThemed() && (pWnd->GetDlgCtrlID()!=IDC_UNCHECKMIGRATED))
	{
		pDC->SetBkColor(0xFFFFFF);

		hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	return hbr;
}

void ReportDlg::OnTabChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CTabCtrl* tabs = (CTabCtrl*)GetDlgItem(IDC_TABS);
	SetPage(tabs->GetCurSel());
}

void ReportDlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	ML_Item* pFolder = m_Lists[m_Page]->m_Items[pItem->iItem];

	if (pItem->mask & LVIF_TEXT)
		pItem->pszText = pFolder->Name;

	if (pItem->mask & LVIF_IMAGE)
		pItem->iImage = pFolder->Icon;
}

void ReportDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		ML_Item* pFolder = m_Lists[m_Page]->m_Items[pNMListView->iItem];

		GetDlgItem(IDC_STATUS1)->SetWindowText(pFolder->Path);

		if (m_Page==0)
		{
			CString tmpStr;
			if (pFolder->List->m_FileCount)
			{
				CString mask;
				ENSURE(mask.LoadString(pFolder->List->m_FileCount==1 ? IDS_REPORTSTATUS_ONEFILE : IDS_REPORTSTATUS_FILES));

				WCHAR tmpBuf[256];
				LFINT64ToString(pFolder->List->m_FileSize, tmpBuf, 256);
				tmpStr.Format(mask, pFolder->List->m_FileCount, tmpBuf);
			}
			else
			{
				ENSURE(tmpStr.LoadString(IDS_REPORTSTATUS_NOFILES));
			}

			GetDlgItem(IDC_STATUS2)->SetWindowText(tmpStr);
		}
		else
		{
			WCHAR* tmpStr = LFGetErrorText(pFolder->List->m_LastError);
			GetDlgItem(IDC_STATUS2)->SetWindowText(tmpStr);
			free(tmpStr);
		}
	}
}
