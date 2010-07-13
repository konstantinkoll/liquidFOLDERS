
// CPaneList.cpp: Implementierung der Klasse CPaneList
//

#include "stdafx.h"
#include "CPaneList.h"


// MyDrawManager
//

class MyDrawManager : CMFCVisualManagerOffice2007
{
public:
	MyDrawManager() : CMFCVisualManagerOffice2007()
	{
	}

	~MyDrawManager()
	{
	}

	void MyDrawManager::DrawItem(CDC* pDC, CRect& rect, BOOL Selected, BOOL Focused)
	{
		m_ctrlMenuHighlighted[Selected ? 0 : 1].Draw(pDC, rect, 0, Focused ? 255 : 175);
	}
};


// CPaneList
//

CPaneList::CPaneList()
	: CListCtrl()
{
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	MenuResID = 0;
	LastWidth = 0;
	Editing = -1;
}

CPaneList::~CPaneList()
{
}


BEGIN_MESSAGE_MAP(CPaneList, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT_EX(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT_EX(LVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()

void CPaneList::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	CMFCVisualManager* dm;

	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		dm = CMFCVisualManager::GetInstance();
		if (!dm->IsKindOf(RUNTIME_CLASS(CMFCVisualManagerOffice2007)))
		{
			*pResult = CDRF_DODEFAULT;
			return;
		}

		DrawItem((int)lplvcd->nmcd.dwItemSpec, CDC::FromHandle(lplvcd->nmcd.hdc), dm);
		*pResult = CDRF_SKIPDEFAULT;
		break;
	default:
		*pResult = CDRF_DODEFAULT;
	}
}

BOOL CPaneList::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	Editing = pDispInfo->item.iItem;
	*pResult = 0;

	return FALSE;
}

BOOL CPaneList::OnEndLabelEdit(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	Editing = -1;
	*pResult = 0;

	return FALSE;
}

void CPaneList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (MenuResID)
	{
		LVHITTESTINFO pInfo;
		if ((point.x<0) || (point.y<0))
		{
			CRect r;
			GetItemRect(GetNextItem(-1, LVNI_FOCUSED), r, LVIR_ICON);
			pInfo.pt.x = r.left;
			pInfo.pt.y = r.top;
			point = pInfo.pt;
			ClientToScreen(&point);
		}
		else
		{
			pInfo.pt = point;
			ScreenToClient(&pInfo.pt);
		}

		SubItemHitTest(&pInfo);
		if (pInfo.iItem>-1)
			if (GetNextItem(pInfo.iItem-1, LVNI_FOCUSED | LVNI_SELECTED)==pInfo.iItem)
			{
				CMenu menu;
				menu.LoadMenu(MenuResID);
				ASSERT_VALID(&menu);

				CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu();
				pPopupMenu->Create(this, point.x, point.y, (HMENU)(*menu.GetSubMenu(0)));

				ASSERT_VALID(GetOwner());
				pPopupMenu->SetMessageWnd(GetOwner());

				return;
			}
	}

	// Ins Leere
	ASSERT_VALID(GetOwner());
	GetOwner()->SendMessage(WM_CONTEXTMENU, 0, MAKELPARAM(point.x, point.y));
}

void CPaneList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar==VK_F2) && (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
	{
		int idx = GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
		if (idx!=-1)
		{
			EditLabel(idx);
			return;
		}
	}

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPaneList::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	ModifyStyle(WS_HSCROLL, 0);
	CListCtrl::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CPaneList::DrawItem(int nID, CDC* pDC, CMFCVisualManager* dm)
{
	CRect rect;
	CImageList* img;
	int H;
	COLORREF oldCol;
	COLORREF bkCol = GetBkColor();

	// Background
	CRect rectBounds;
	GetItemRect(nID, rectBounds, LVIR_BOUNDS);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	UINT State = GetItemState(nID, LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT);
	if ((State & (LVIS_SELECTED | LVIS_FOCUSED)) && ((State & LVIS_SELECTED) || ((GetFocus()==this) && (!(GetExtendedStyle() & LVS_EX_ONECLICKACTIVATE))) || (GetStyle() & LVS_SHOWSELALWAYS)))
	{
		((MyDrawManager*)dm)->DrawItem(pDC, rectBounds, State & LVIS_SELECTED, ((GetFocus()==this) && (State & LVIS_FOCUSED)));
	}
	else
	{
		if (!(GetExtendedStyle() & LVS_EX_ONECLICKACTIVATE))
			pDC->FillSolidRect(rectBounds, bkCol);
	}

	TCHAR text[MAX_PATH];
	UINT columns[2];
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.iItem = nID;
	item.pszText = text;
	item.cchTextMax = sizeof(text)/sizeof(TCHAR);
	item.puColumns = columns;
	item.cColumns = 2;
	item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_COLUMNS;
	GetItem(&item);

	// Icon
	if (item.iImage!=-1)
	{
		IMAGEINFO ImageInfo;
		img = GetImageList(LVSIL_NORMAL);
		if (img)
			if (img->GetImageInfo(item.iImage, &ImageInfo))
			{
				int w = ImageInfo.rcImage.right-ImageInfo.rcImage.left;
				int h = ImageInfo.rcImage.bottom-ImageInfo.rcImage.top;
				GetItemRect(nID, &rect, LVIR_ICON);
				if (GetView()==LV_VIEW_TILE)
				{
					rect.OffsetRect(4, (rectBounds.Height()-h)/2);
				}
				else
				{
					rect.OffsetRect((rectBounds.Width()-w)/2, 4);
				}
				img->DrawEx(pDC, item.iImage, rect.TopLeft(), CSize(w, h), CLR_NONE, 0xFFFFFF, State & LVIS_CUT ? GetView()==LV_VIEW_TILE ? ILD_BLEND50 : ILD_BLEND25 : ILD_TRANSPARENT);
			}
	}

	// Label
	if (Editing!=item.iItem)
	{
		H = pDC->GetTextExtent(item.pszText).cy;
		GetItemRect(nID, &rect, LVIR_LABEL);

		UINT nFormat;
		if (GetView()==LV_VIEW_TILE)
		{
			rect.DeflateRect(2, 1);
			rect.OffsetRect(0, (rectBounds.Height()-H*3)/2-1);
			nFormat = DT_SINGLELINE;
		}
		else
		{
			rect.bottom = rectBounds.bottom-1;
			nFormat = DT_CENTER | DT_WORDBREAK;
		}
		pDC->DrawText(item.pszText, -1, rect, DT_NOPREFIX | DT_END_ELLIPSIS | nFormat);

		if (GetView()==LV_VIEW_TILE)
		{
			item.mask = LVIF_TEXT;

			oldCol = (COLORREF)-1;
			if ((!State) && (pDC->GetTextColor()==0x000000))
				oldCol = pDC->SetTextColor((bkCol>>1) & 0x7F7F7F);

			for (UINT a=0; a<item.cColumns; a++)
			{
				rect.top += pDC->GetTextExtent(item.pszText).cy;

				item.iSubItem = item.puColumns[a];
				item.pszText = text;
				GetItem(&item);

				pDC->DrawText(item.pszText, -1, rect, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
			}

			if (oldCol!=(COLORREF)-1)
				pDC->SetTextColor(oldCol);
		}
	}

	pDC->SetBkMode(oldMode);
}

BOOL CPaneList::SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{

	if (cx<LastWidth)
	{
		SetTileSize(cx);
		return CListCtrl::SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);
	}
	else
	{
		BOOL res = CListCtrl::SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);
		SetTileSize(cx);
		return res;
	}
}

void CPaneList::SetTileSize(int cx)
{
	if (cx==-1)
	{
		cx = LastWidth;
	}
	else
	{
		LastWidth = cx;
	}

	if (GetStyle() & WS_VSCROLL)
		cx -= GetSystemMetrics(SM_CXVSCROLL);

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 2;
	tvi.dwFlags = LVTVIF_FIXEDSIZE;
	tvi.dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS | LVTVIM_LABELMARGIN;
	tvi.sizeTile.cx = cx;
	tvi.sizeTile.cy = 60;
	tvi.rcLabelMargin.bottom = 1;
	tvi.rcLabelMargin.top = 1;
	tvi.rcLabelMargin.left = 1;
	tvi.rcLabelMargin.right = 1;

	if (IsGroupViewEnabled() && (osInfo.dwMajorVersion==5))
		tvi.sizeTile.cx -= 16;

	SetTileViewInfo(&tvi);
}

void CPaneList::SetContextMenu(UINT _MenuResID)
{
	MenuResID = _MenuResID;
}

void CPaneList::EnableGroupView(BOOL fEnable)
{
	CListCtrl::EnableGroupView(fEnable);

	if (osInfo.dwMajorVersion==5)
		SetTileSize(-1);
}
