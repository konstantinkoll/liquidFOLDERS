
#include "stdafx.h"
#include "HistoryWnd.h"
#include "Resource.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"


// Breadcrumbs
//

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, FVPersistentData& data)
{
	BreadcrumbItem* add = new BreadcrumbItem;
	add->next = *bi;
	add->filter = f;
	add->data = data;
	*bi = add;
}

void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, FVPersistentData* data)
{
	*f = NULL;
	ZeroMemory(data, sizeof(FVPersistentData));

	if (*bi)
	{
		*f = (*bi)->filter;
		*data = (*bi)->data;
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		delete victim;
	}
}

void DeleteBreadcrumbs(BreadcrumbItem** bi)
{
	while (*bi)
	{
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		LFFreeFilter(victim->filter);
		delete victim;
	}
}


// CHistoryWnd
//

CHistoryWnd::CHistoryWnd()
{
	m_CurrentItem = -1;
}

CHistoryWnd::~CHistoryWnd()
{
}

void CHistoryWnd::AddFilterItem(LFFilter* f, BOOL append, BOOL focus)
{
	UINT puColumns[] = { 1, 2 };
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS;
	lvi.cColumns = 2;
	lvi.puColumns = puColumns;
	lvi.iItem = append ? m_wndList.GetItemCount() : 0;
	lvi.pszText = f->Name;
	lvi.iImage = f->Result.FilterType;
	INT idx = m_wndList.InsertItem(&lvi);

	TCHAR dateStr[256];
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &f->Result.Time, NULL, dateStr, sizeof(dateStr)/sizeof(TCHAR));
	TCHAR timeStr[256];
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &f->Result.Time, NULL, timeStr, sizeof(timeStr)/sizeof(TCHAR));
	CString tmpStr;
	tmpStr = dateStr;
	tmpStr += L", ";
	tmpStr += timeStr;
	m_wndList.SetItemText(idx, 2, tmpStr);

	CString Hint;
	CString Mask;
	if (f->Mode==LFFilterModeStores)
	{
		ENSURE(Mask.LoadString(f->Result.StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
		Hint.Format(Mask, f->Result.StoreCount);
	}
	else
	{
		WCHAR tmpBuf[256];
		ENSURE(Mask.LoadString(f->Result.FileCount==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
		LFINT64ToString(f->Result.FileSize, tmpBuf, 256);
		Hint.Format(Mask, f->Result.FileCount, tmpBuf);
	}

	m_wndList.SetItemText(idx, 1, Hint);

	if (focus)
	{
		m_wndList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_CurrentItem = idx;
	}
}

void CHistoryWnd::UpdateList(BreadcrumbItem* prev, LFFilter* current, BreadcrumbItem* next)
{
	m_wndList.SetRedraw(FALSE);
	m_wndList.DeleteAllItems();

	// Zurück
	while (prev)
	{
		AddFilterItem(prev->filter, FALSE);
		prev = prev->next;
	}

	// Aktuell
	AddFilterItem(current, TRUE, TRUE);

	// Vorwärts
	while (next)
	{
		AddFilterItem(next->filter, TRUE);
		next = next->next;
	}

	m_wndList.SetRedraw(TRUE);
}


BEGIN_MESSAGE_MAP(CHistoryWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_COMMAND(ID_NAV_GOTOHISTORY, OnGotoHistory)
	ON_COMMAND(IDOK, OnGotoHistory)
	ON_NOTIFY(NM_DBLCLK, ID_HISTORYLIST, OnNotifyGotoHistory)
END_MESSAGE_MAP()

INT CHistoryWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct)==-1)
		return -1;
	SetRedraw(FALSE);

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
	if (!m_wndList.Create(dwViewStyle, CRect(0, 0, 0, 0), this, ID_HISTORYLIST))
		return -1;

	const DWORD dwExStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP;
	m_wndList.SetExtendedStyle(dwExStyle);

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	for (UINT a=0; a<3; a++)
	{
		lvc.pszText = L"";
		lvc.fmt = LVCFMT_LEFT;
		lvc.iSubItem = a;
		m_wndList.InsertColumn(a, &lvc);
	}

	m_wndList.SetView(LV_VIEW_TILE);

	m_Icons.Create(IDB_HISTORYICONS, NULL, 0, 7, 32, 32);
	m_wndList.SetImageList(&m_Icons, LVSIL_NORMAL);

	SetRedraw(TRUE);
	return 0;
}

void CHistoryWnd::OnSize(UINT nType, INT cx, INT cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	m_wndList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CHistoryWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndList.SetFocus();
}

void CHistoryWnd::OnPaint()
{
	m_wndList.SetBkColor(afxGlobalData.clrBarFace);
	m_wndList.SetTextBkColor(afxGlobalData.clrBarFace);

	CDockablePane::OnPaint();
}

void CHistoryWnd::OnGotoHistory()
{
	CFrameWnd* w = GetParentFrame();
	INT idx = m_wndList.GetNextItem(-1, LVNI_FOCUSED);

	if ((idx>-1) && (m_CurrentItem>-1) && (w))
		if (idx==m_CurrentItem)
		{
			// Aktueller Filter
			w->SendMessage(WM_COMMAND, ID_NAV_RELOAD);
		}
		else
			if (idx<m_CurrentItem)
			{
				// Zurück
				w->SendMessage(ID_NAV_BACK, (WPARAM)(m_CurrentItem-idx));
			}
			else
			{
				// Vor
				w->SendMessage(ID_NAV_FORWARD, (WPARAM)(idx-m_CurrentItem));
			}
}

void CHistoryWnd::OnNotifyGotoHistory(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnGotoHistory();
}
