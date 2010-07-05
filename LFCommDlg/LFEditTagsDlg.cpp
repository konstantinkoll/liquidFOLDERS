#include "StdAfx.h"
#include "LFEditTagsDlg.h"
#include "Resource.h"

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFEditTagsDlg::LFEditTagsDlg(CWnd* pParentWnd, CString _Tags)
	: CDialog(IDD_EDITTAGS, pParentWnd)
{
	m_Tags = _Tags;
	p_App = (LFApplication*)AfxGetApp();
}

LFEditTagsDlg::~LFEditTagsDlg()
{
}

BEGIN_MESSAGE_MAP(LFEditTagsDlg, CDialog)
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
	GetDlgItem(IDC_TAGS)->SetWindowText(m_Tags);

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
	m_TagList.SetExtendedStyle(LVS_EX_DOUBLEBUFFER);

	// Vorschläge
	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeDirectoryTree;
	f->Options.IgnoreSlaves = true;
	LFSearchResult* res = LFQuery(f);
	LFGroupSearchResult(res, LFAttrTags, false, false, 0, true, f);
	LFFreeFilter(f);

	for (unsigned int a=0; a<res->m_ItemCount; a++)
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
			int idx = m_TagList.InsertItem(&lvi);

			CString cnt;
			cnt.Format(_T("%d"), i->AggregateCount);
			m_TagList.SetItemText(idx, 1, cnt);
		}
	}

	LFFreeSearchResult(res);

	return TRUE;
}

void LFEditTagsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TAGLIST, m_TagList);

	if (pDX->m_bSaveAndValidate)
		GetDlgItem(IDC_TAGS)->GetWindowText(m_Tags);
}
