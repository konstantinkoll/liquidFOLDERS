
// CFileList.cpp: Implementierung der Klasse CFileList
//

#include "stdafx.h"
#include "CFileList.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CFileList
//

CFileList::CFileList()
	: CExplorerList()
{
	OwnerData = FALSE;
	Editing = FALSE;
	ItemChanged = 0;
	LastSortBy = -1;
	hTheme = NULL;
	ColumnCount = 0;
}

CFileList::~CFileList()
{
}

void CFileList::Create(CFileView* pViewWnd, BOOL _OwnerData)
{
	View = pViewWnd;
	OwnerData = _OwnerData;

	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE |
		LVS_EDITLABELS | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP;
	if (_OwnerData)
		dwStyle |= LVS_OWNERDATA;

	CRect rect;
	rect.SetRectEmpty();
	CExplorerList::Create(dwStyle, rect, pViewWnd, 1);
	SetExtendedStyle(FileListExtendedStyles);
}

void CFileList::SetHeader(BOOL sorting, BOOL selectCol)
{
	CHeaderCtrl* pHdr = GetHeaderCtrl();
	if (!pHdr)
		return;

	pHdr->SetRedraw(FALSE);

	// Spalten
	if (!sorting)
		CreateColumns();

	// Alte Markierung löschen
	if ((LastSortBy!=(int)View->pViewParameters->SortBy) && (LastSortBy!=-1))
	{
		HDITEM hdi;
		hdi.mask = HDI_FORMAT;

		int col = FindColumn(LastSortBy);
		if (col!=-1)
		{
			pHdr->GetItem(col, &hdi);
			hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
			pHdr->SetItem(col, &hdi);
		}
	}

	// Markierung setzen
	int col = FindColumn(View->pViewParameters->SortBy);
	if (col!=-1)
	{
		HDITEM hdi;
		hdi.mask = HDI_FORMAT;

		pHdr->GetItem(col, &hdi);
		hdi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
		hdi.fmt |= View->pViewParameters->Descending ? HDF_SORTDOWN : HDF_SORTUP;
		pHdr->SetItem(col, &hdi);
	}

	LastSortBy = View->pViewParameters->SortBy;

	// Spalte markieren
	if (selectCol)
		SetSelectedColumn(col);

	pHdr->SetRedraw(TRUE);
	pHdr->Invalidate();
}

BOOL CFileList::SetColumnWidth(int nCol, int cx)
{
	if (theApp.m_Attributes[ColumnMapping[nCol]]->Type==LFTypeRating)
		cx = RatingBitmapWidth+12;

	return CExplorerList::SetColumnWidth(nCol, cx);
}

void CFileList::CreateColumns()
{
	for (UINT a=0; a<ColumnCount; a++)
		DeleteColumn(0);
	ColumnCount = 0;

	switch (GetView())
	{
	case LV_VIEW_DETAILS:
		for (UINT a=0; a<LFAttributeCount; a++)
			if (View->pViewParameters->ColumnWidth[a])
				AddColumn(a);

		SetColumnOrderArray(ColumnCount, View->pViewParameters->ColumnOrder);
		break;
	case LV_VIEW_TILE:
		AddColumn(LFAttrFileName);
		AddColumn(LFAttrComment);
		AddColumn(LFAttrFileTime);
		AddColumn(LFAttrHint);
		AddColumn(LFAttrFileSize);
		break;
	default:
		AddColumn(LFAttrFileName);
	}
}

void CFileList::AddColumn(UINT attr)
{
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = theApp.m_Attributes[attr]->Name;
	lvc.fmt = ((theApp.m_Attributes[attr]->Type >= LFTypeUINT) || (attr==LFAttrStoreID) || (attr==LFAttrFileID)) ? LVCFMT_RIGHT : LVCFMT_LEFT;
	lvc.cx = View->pViewParameters->ColumnWidth[attr];
	if (theApp.m_Attributes[attr]->Type==LFTypeRating)
	{
		lvc.cx = RatingBitmapWidth+12;
		lvc.fmt |= LVCFMT_NO_DPI_SCALE;
	}

	ColumnMapping[ColumnCount] = lvc.iSubItem = attr;
	InsertColumn(ColumnCount++, &lvc);
}


int CFileList::FindColumn(UINT attr)
{
	for (UINT a=0; a<ColumnCount; a++)
		if (ColumnMapping[a]==attr)
			return a;

	return -1;
}

BEGIN_MESSAGE_MAP(CFileList, CExplorerList)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY(HDN_BEGINTRACK, 0, OnHeaderCanResize)
	ON_NOTIFY(HDN_ENDTRACK, 0, OnHeaderResize)
	ON_NOTIFY(HDN_BEGINDRAG, 0, OnHeaderCanReorder)
	ON_NOTIFY(HDN_ENDDRAG, 0, OnHeaderReorder)
	ON_WM_SIZE()
END_MESSAGE_MAP()


void CFileList::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &pDispInfo->item;

	int idx = pItem->iItem;

	if (pItem->mask & LVIF_COLUMNS)
	{
		pItem->cColumns = 3;
		pItem->puColumns[0] = 1;
		pItem->puColumns[1] = 2;
		pItem->puColumns[2] = 4;
	}

	if (View->result)
	{
		UINT attr = ColumnMapping[pItem->iSubItem];
		if ((pItem->mask & LVIF_TEXT) && (theApp.m_Attributes[attr]->Type!=LFTypeRating))
		{
			LFAttributeToString(View->result->m_Items[idx], attr, m_StrBuffer, 256);
			pItem->pszText = (LPWSTR)m_StrBuffer;
		}
		if (pItem->mask & LVIF_IMAGE)
			pItem->iImage = 5;//View->result->m_Items[idx]->IconID-1;
	}
}

void CFileList::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	int col;
	UINT attr;

	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if ((hTheme) && (GetItemState((int)lplvcd->nmcd.dwItemSpec, LVIS_SELECTED)))
			lplvcd->nmcd.uItemState &= ~CDIS_FOCUS;
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
		*pResult = CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_ITEMPOSTPAINT|CDDS_SUBITEM:
		col = lplvcd->iSubItem;
		attr = ColumnMapping[col];
		if (theApp.m_Attributes[attr]->Type==LFTypeRating)
		{
			CRect rect;
			GetSubItemRect((int)lplvcd->nmcd.dwItemSpec, col, LVIR_BOUNDS, rect);
			UINT iState = GetItemState((int)lplvcd->nmcd.dwItemSpec, LVIS_SELECTED);
			BOOL hot = GetHotItem()==(int)lplvcd->nmcd.dwItemSpec;

			COLORREF clr = GetBkColor();
			if ((GetExtendedStyle() & LVS_EX_FULLROWSELECT) && (!hTheme) && (iState))
				clr = GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE);
			HBRUSH brsh = CreateSolidBrush(clr);
			FillRect(lplvcd->nmcd.hdc, rect, brsh);
			DeleteObject(brsh);

			if ((GetExtendedStyle() & LVS_EX_FULLROWSELECT) && (hTheme) && (hot || iState))
			{
				CRect rectBounds;
				GetItemRect((int)lplvcd->nmcd.dwItemSpec, rectBounds, LVIR_BOUNDS);

				const int StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
				theApp.zDrawThemeBackground(hTheme, lplvcd->nmcd.hdc, LVP_LISTITEM, StateIDs[iState | hot], rectBounds, rect);
			}

			HDC hdcMem = CreateCompatibleDC(lplvcd->nmcd.hdc);
			UCHAR level = *(UCHAR*)View->result->m_Items[lplvcd->nmcd.dwItemSpec]->AttributeValues[attr];
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, attr==LFAttrRating ? theApp.m_RatingBitmaps[level] : theApp.m_PriorityBitmaps[level]);

			int w = min(rect.Width(), RatingBitmapWidth);
			int h = min(rect.Height(), RatingBitmapHeight);
			BLENDFUNCTION LF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
			AlphaBlend(lplvcd->nmcd.hdc, rect.left, rect.top+(rect.Height()-h)/2, w, h, hdcMem, 0, 0, w, h, LF);

			SelectObject(hdcMem, hbmOld);
			DeleteDC(hdcMem);

			*pResult = CDRF_SKIPDEFAULT;
		}
		else
		{
			*pResult = CDRF_DODEFAULT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
	}
}

void CFileList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!View->HandleDefaultKeys(nChar, nRepCnt, nFlags))
	{
		ItemChanged = 1;
		CExplorerList::OnKeyDown(nChar, nRepCnt, nFlags);

		if (ItemChanged & 2)
			GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);

		ItemChanged = 0;
	}
}

void CFileList::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	GetParentFrame()->SendMessage(WM_COMMAND, ID_NAV_STARTNAVIGATION);
}

void CFileList::OnLButtonDown(UINT nFlags, CPoint point)
{
	ItemChanged = 1;
	CExplorerList::OnLButtonDown(nFlags, point);

	if (ItemChanged & 2)
		GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);

	ItemChanged = 0;
}

void CFileList::OnRButtonDown(UINT nFlags, CPoint point)
{
	ItemChanged = 1;
	CExplorerList::OnRButtonDown(nFlags, point);

	if (ItemChanged & 2)
		GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);

	ItemChanged = 0;
}

void CFileList::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// Auf den Spalten-Header ?
	if (pWnd==GetHeaderCtrl())
	{
		CMenu menu;
		if (menu.CreatePopupMenu())
		{
			// Attributliste
			for (UINT a=0; a<=LFLastLocalAttribute; a++)
				if (theApp.m_Contexts[View->ActiveContextID]->AllowedAttributes->IsSet(a))
					menu.AppendMenu(MF_BYPOSITION | MF_STRING, ID_TOGGLE_ATTRIBUTE+a, theApp.m_Attributes[a]->Name);
			menu.AppendMenu(MF_SEPARATOR);

			CMenu more;
			if (more.CreateMenu())
			{
				// Attributliste
				for (UINT a=LFLastLocalAttribute+1; a<LFAttributeCount; a++)
					if (theApp.m_Contexts[View->ActiveContextID]->AllowedAttributes->IsSet(a))
						more.AppendMenu(MF_BYPOSITION | MF_STRING, ID_TOGGLE_ATTRIBUTE+a, theApp.m_Attributes[a]->Name);

				if (more.GetMenuItemCount())
				{
					CString tmpStr;
					ENSURE(tmpStr.LoadString(IDS_MOREATTRIBUTES));
					menu.AppendMenu(MF_STRING | MF_POPUP, (UINT_PTR)(HMENU)more, tmpStr);
					menu.AppendMenu(MF_SEPARATOR);
				}
			}

			// Autosize
			CString tmpStr;
			ENSURE(tmpStr.LoadString(ID_VIEW_AUTOSIZECOLUMNS));
			menu.AppendMenu(MF_BYPOSITION | MF_STRING, ID_VIEW_AUTOSIZECOLUMNS, tmpStr);

			// Andere
			View->AppendContextMenu(&menu);

			CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu();
			pPopupMenu->Create(this, point.x, point.y, (HMENU)menu);
			return;
		}
	}

	// Auf eine Datei ?
	LVHITTESTINFO pInfo;
	if ((point.x<0) || (point.y<0))
	{
		CRect r;
		GetItemRect(GetNextItem(-1, LVNI_FOCUSED), r, LVIR_ICON);
		pInfo.pt.x = r.left;
		pInfo.pt.y = r.top;
		point = pInfo.pt;
	}
	else
	{
		ScreenToClient(&point);
		pInfo.pt = point;
	}

	SubItemHitTest(&pInfo);
	if (pInfo.iItem>-1)
		if (GetNextItem(pInfo.iItem-1, LVNI_FOCUSED | LVNI_SELECTED)==pInfo.iItem)
		{
			ClientToScreen(&point);
			View->OnItemContextMenu(pInfo.iItem, point);
			return;
		}

	// Ins Leere
	ClientToScreen(&point);
	View->OnContextMenu(point);
}

void CFileList::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if (View->result->m_Items[pDispInfo->item.iItem]->Type & (LFTypeVirtual | LFTypeDrive))
	{
		*pResult = TRUE;
		return;
	}

	Editing = TRUE;
	*pResult = FALSE;
}

void CFileList::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	Editing = FALSE;
	*pResult = ((CMainFrame*)GetParentFrame())->RenameSingleItem(pDispInfo->item.iItem, pDispInfo->item.pszText);
}

void CFileList::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		if (ItemChanged & 1)
		{
			ItemChanged |= 2;
		}
		else
		{
			GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);
			ItemChanged = 0;
		}
}

void CFileList::OnColumnClick(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNM = (NM_LISTVIEW*)pNMHDR;
	UINT attr = ColumnMapping[pNM->iSubItem];

	if ((int)View->pViewParameters->SortBy==attr)
	{
		View->pViewParameters->Descending ^= 1;
	}
	else
	{
		if (!LFAttributeSortableInView(attr, View->m_ViewParameters.Mode))
		{
			CString msg;
			ENSURE(msg.LoadString(IDS_ATTRIBUTENOTSORTABLE));
			MessageBox(msg, theApp.m_Attributes[attr]->Name, MB_OK | MB_ICONWARNING);

			return;
		}

		View->pViewParameters->SortBy = attr;
		View->pViewParameters->Descending = (theApp.m_Attributes[attr]->Type==LFTypeRating);
	}

	theApp.UpdateSortOptions(View->ActiveContextID);
}

void CFileList::OnHeaderCanResize(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
		*pResult = (theApp.m_Attributes[ColumnMapping[pHdr->iItem]]->Type==LFTypeRating) ||
					(View->m_ViewParameters.ColumnWidth[ColumnMapping[pHdr->iItem]]==0);
}

void CFileList::OnHeaderResize(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
	{
		if (pHdr->pitem->cxy<32)
		{
			pHdr->pitem->cxy = (ColumnMapping[pHdr->iItem]==LFAttrFileName) ? 32 : 0;
			SetColumnWidth(pHdr->iItem, pHdr->pitem->cxy);
		}

		View->pViewParameters->ColumnWidth[ColumnMapping[pHdr->iItem]] = pHdr->pitem->cxy;
		View->OnViewOptionsChanged(TRUE);

		*pResult = FALSE;
	}
}

void CFileList::OnHeaderCanReorder(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;
	*pResult = (ColumnMapping[pHdr->iItem]==LFAttrFileName);
}

void CFileList::OnHeaderReorder(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_ORDER)
	{
		if (pHdr->pitem->iOrder==LFAttrFileName)
			pHdr->pitem->iOrder = 1;
		if (pHdr->iItem==LFAttrFileName)
			pHdr->pitem->iOrder = 0;

		// GetColumnOrderArray() enthält noch die alte Reihenfolge, daher:
		// 1. Spalte an der alten Stelle löschen
		for (UINT a=0; a<ColumnCount; a++)
			if (View->pViewParameters->ColumnOrder[a]==pHdr->iItem)
			{
				for (UINT b=a; b<ColumnCount-1; b++)
					View->pViewParameters->ColumnOrder[b] = View->pViewParameters->ColumnOrder[b+1];
				break;
			}

		// 2. Spalte an der neuen Stelle einfügen
		for (int a=ColumnCount-1; a>pHdr->pitem->iOrder; a--)
			View->pViewParameters->ColumnOrder[a] = View->pViewParameters->ColumnOrder[a-1];
		View->pViewParameters->ColumnOrder[pHdr->pitem->iOrder] = pHdr->iItem;

		View->OnViewOptionsChanged();
		*pResult = FALSE;
	}
}

void CFileList::OnSize(UINT nType, int cx, int cy)
{
	CExplorerList::OnSize(nType, cx, cy);
	Invalidate();
}
