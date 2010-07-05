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
	tvi.sizeTile.cx = 140;
	tvi.sizeTile.cy = 40;

	m_TagList.SetTileViewInfo(&tvi);
	m_TagList.SetView(LV_VIEW_TILE);
	m_TagList.SetExtendedStyle(LVS_EX_DOUBLEBUFFER);

	// Vorschläge
	for (unsigned int a=0; a<1000; a++)
	{
		UINT puColumns[] = { 1, 2 };
		LVITEM lvi;
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_TEXT | LVIF_COLUMNS;
		lvi.cColumns = 1;
		lvi.puColumns = puColumns;
		lvi.iItem = a;
		lvi.pszText = _T("Test");
		int idx = m_TagList.InsertItem(&lvi);

		m_TagList.SetItemText(idx, 1, _T("0"));
	}

	return TRUE;
}

void LFEditTagsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_TAGLIST, m_TagList);

	if (pDX->m_bSaveAndValidate)
		GetDlgItem(IDC_TAGS)->GetWindowText(m_Tags);
}
