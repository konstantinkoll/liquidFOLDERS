
// LFEditTagsDlg.cpp: Implementierung der Klasse LFEditTagsDlg
//

#include "StdAfx.h"
#include "LFEditTagsDlg.h"
#include "Resource.h"


// LFEditTagsDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFEditTagsDlg::LFEditTagsDlg(CWnd* pParentWnd, CString _Tags, CHAR* _StoreID)
	: CDialog(IDD_EDITTAGS, pParentWnd)
{
	m_Tags = _Tags;
	p_App = (LFApplication*)AfxGetApp();

	StoreIDValid = (_StoreID!=NULL);
	if (_StoreID)
		strcpy_s(StoreID, LFKeySize, _StoreID);
}


BEGIN_MESSAGE_MAP(LFEditTagsDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_TAGLIST, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_TAGLIST, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TAGLIST, OnItemChanged)
	ON_BN_CLICKED(IDC_ADDTAGS, OnAddTags)
END_MESSAGE_MAP()


BOOL LFEditTagsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_EDITTAGS));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Eingabezeile
	m_TagEdit.SetWindowText(m_Tags);
	m_TagEdit.SetLimitText(255);

	// Liste
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = _T("");
	lvc.iSubItem = 0;
	m_TagList.InsertColumn(0, &lvc);
	lvc.iSubItem = 1;
	m_TagList.InsertColumn(1, &lvc);

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 2;
	tvi.dwFlags = LVTVIF_FIXEDSIZE;
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tvi.sizeTile.cx = 152;
	tvi.sizeTile.cy = 24;

	m_TagList.SetTileViewInfo(&tvi);
	m_TagList.SetView(LV_VIEW_TILE);
	m_TagList.SetExtendedStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_ONECLICKACTIVATE);
	m_TagList.SetHoverTime(1);

	if (StoreIDValid)
	{
		// Vorschläge
		LFFilter* f = LFAllocFilter();
		strcpy_s(f->StoreID, LFKeySize, StoreID);
		f->Mode = LFFilterModeDirectoryTree;
		f->Options.IgnoreSlaves = true;
		LFSearchResult* base = LFQuery(f);
		LFSearchResult* res = LFGroupSearchResult(base, LFAttrTags, false, false, 0, true, f);
		LFFreeSearchResult(base);
		LFFreeFilter(f);

		for (UINT a=0; a<res->m_ItemCount; a++)
		{
			LFItemDescriptor* i = res->m_Items[a];
			if (((i->Type & LFTypeMask)==LFTypeVirtual) && (i->AggregateCount))
			{
				UINT puColumns[] = { 1, 2 };
				LVITEM lvi;
				ZeroMemory(&lvi, sizeof(lvi));
				lvi.mask = LVIF_TEXT | LVIF_COLUMNS;
				lvi.cColumns = 1;
				lvi.puColumns = puColumns;
				lvi.iItem = a;
				lvi.pszText = i->CoreAttributes.FileName;
				INT idx = m_TagList.InsertItem(&lvi);

				CString cnt;
				cnt.Format(_T("%d"), i->AggregateCount);
				m_TagList.SetItemText(idx, 1, cnt);
			}
		}

		LFFreeSearchResult(res);
	}
	else
	{
		m_TagList.EnableWindow(FALSE);
	}

	return TRUE;
}

void LFEditTagsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_TAGS, m_TagEdit);
	DDX_Control(pDX, IDC_TAGLIST, m_TagList);

	if (pDX->m_bSaveAndValidate)
		GetDlgItem(IDC_TAGS)->GetWindowText(m_Tags);
}

void LFEditTagsDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnAddTags();
}

void LFEditTagsDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		GetDlgItem(IDC_ADDTAGS)->EnableWindow(m_TagList.GetNextItem(-1, LVNI_SELECTED)!=-1);
}

void LFEditTagsDlg::OnAddTags()
{
	CString Tags;
	m_TagEdit.GetWindowText(Tags);

	INT idx = m_TagList.GetNextItem(-1, LVNI_SELECTED);
	while (idx!=-1)
	{
		CString toAdd = m_TagList.GetItemText(idx, 0);

		if (Tags!=_T(""))
			Tags += _T(" ");
		Tags += toAdd;

		m_TagList.SetItemState(idx, 0, LVIS_SELECTED);
		idx = m_TagList.GetNextItem(idx, LVNI_SELECTED);
	}

	WCHAR tmpStr[512];
	wcscpy_s(tmpStr, 512, Tags);
	LFSanitizeUnicodeArray(tmpStr, 512);

	m_TagEdit.SetWindowText(tmpStr);
	m_TagEdit.SetSel(Tags.GetLength(), Tags.GetLength());
	m_TagEdit.SetFocus();
}
