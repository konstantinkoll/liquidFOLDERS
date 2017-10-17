
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"
#include "CListView.h"
#include "liquidFOLDERS.h"


// CListView
//

#define GetItemData(Index)     ((FVItemData*)(m_pItemData+Index*m_DataSize))
#define GUTTER                 BACKSTAGEBORDER
#define ITEMPADDING            2
#define MAXAUTOWIDTH           400
#define PREVIEWITEMOFFSET      2
#define PREVIEWWIDTH           128+CARDPADDING
#define SPACER                 (4*ITEMPADDING+1)

CListView::CListView()
	: CFileView()
{
	m_pFolderItems = NULL;
	m_HeaderItemClicked = -1;
	m_HasFolders = m_IgnoreHeaderItemChange = FALSE;
}

void CListView::SetViewSettings(BOOL UpdateSearchResultPending)
{
	// Copy context view settings early for header
	m_ContextViewSettings = *p_ContextViewSettings;

	AdjustHeader();

	// Commit settings
	if (p_CookedFiles && !UpdateSearchResultPending)
	{
		AdjustLayout();
		RedrawWindow();
	}
}

void CListView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	// Group search result
	if (m_pFolderItems)
		LFFreeSearchResult(m_pFolderItems);

	if (p_RawFiles)
	{
		ValidateAllItems();

		// Folders
		UINT TypeMask = 0;

		switch (m_Context)
		{
		case LFContextSubfolderAlbum:
			m_HasFolders = FALSE;
			break;

		case LFContextSubfolderArtist:
			TypeMask = (1<<LFTypeDuration) | (1<<LFTypeGenre);
			m_HasFolders = (theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.IconID && (m_ContextViewSettings.SortBy!=LFAttrArtist));

			break;

		case LFContextSubfolderGenre:
			TypeMask = (1<<LFTypeDuration);
			m_HasFolders = (theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.IconID && (m_ContextViewSettings.SortBy!=LFAttrGenre));

			break;

		default:
			TypeMask = (1<<LFTypeDuration) | (1<<LFTypeGenre) | (1<<LFTypeGeoCoordinates) | (1<<LFTypeIATACode) | (1<<LFTypeRating) | (1<<LFTypeColor) | (1<<LFTypeTime) | (1<<LFTypeApplication);
			m_HasFolders = theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.IconID || (m_ContextViewSettings.SortBy==LFAttrLocationName);
		}

		m_HasFolders |= (TypeMask & (1 << theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.Type));

		// Prepare search result
		if (m_HasFolders)
		{
			m_pFolderItems = LFGroupSearchResult(p_RawFiles, m_ContextViewSettings.SortBy, m_ContextViewSettings.Descending, TRUE, pFilter);
		}
		else
		{
			m_pFolderItems = NULL;

			LFSortSearchResult(p_RawFiles, m_ContextViewSettings.SortBy, m_ContextViewSettings.Descending);
		}

		// Preview attribute
		const UINT Attr = m_ContextViewSettings.SortBy;
		const UINT Type = theApp.m_Attributes[Attr].AttrProperties.Type;

		m_PreviewAttribute = m_HasFolders &&
			((Type==LFTypeGenre) || (Type==LFTypeGeoCoordinates) || (Type==LFTypeIATACode) || (Type==LFTypeApplication) || 
			(Attr==LFAttrRoll) || theApp.m_Attributes[Attr].AttrProperties.ShowRepresentativeThumbnail)
			? Attr : -1;
	}
	else
	{
		m_pFolderItems = NULL;
		m_PreviewAttribute = -1;
	}

	AdjustHeader();
}

void CListView::AdjustHeader()
{
	if (p_CookedFiles)
		if (p_CookedFiles->m_ItemCount)
		{
			m_IgnoreHeaderItemChange = TRUE;
			SetRedraw(FALSE);

			// Set column order (preview column is always first)
			INT ColumnOrder[LFAttributeCount];
			UINT Index = 0;

			if (m_PreviewAttribute>=0)
				ColumnOrder[Index++] = m_PreviewAttribute;

			for (UINT a=0; a<LFAttributeCount; a++)
				if (p_ContextViewSettings->ColumnOrder[a]!=m_PreviewAttribute)
					ColumnOrder[Index++] = p_ContextViewSettings->ColumnOrder[a];

			VERIFY(m_wndHeader.SetOrderArray(LFAttributeCount, ColumnOrder));

			// Set column properties
			for (UINT a=0; a<LFAttributeCount; a++)
			{
				HDITEM hdi;
				hdi.mask = HDI_WIDTH | HDI_FORMAT;
				hdi.cxy = ((INT)a==m_PreviewAttribute) ? PREVIEWWIDTH : p_ContextViewSettings->ColumnWidth[a];
				hdi.fmt = theApp.m_Attributes[a].TypeProperties.FormatRight ? HDF_RIGHT : HDF_LEFT;

				if (p_ContextViewSettings->SortBy==a)
					hdi.fmt |= p_ContextViewSettings->Descending ? HDF_SORTDOWN : HDF_SORTUP;

				if (theApp.m_Attributes[a].TypeProperties.DefaultColumnWidth && hdi.cxy)
				{
					if (theApp.m_Attributes[a].AttrProperties.Type==LFTypeRating)
						{
							hdi.cxy = p_ContextViewSettings->ColumnWidth[a] = RATINGBITMAPWIDTH+SPACER;
						}
					else
					{
						if (hdi.cxy<GetMinColumnWidth(a))
							p_ContextViewSettings->ColumnWidth[a] = hdi.cxy = GetMinColumnWidth(a);
					}
				}
				else
				{
					hdi.cxy = 0;
				}

				m_wndHeader.SetItem(a, &hdi);
			}

			m_wndHeader.ModifyStyle(HDS_HIDDEN, 0);
			SetRedraw(TRUE);
			m_wndHeader.Invalidate();

			m_IgnoreHeaderItemChange = FALSE;

			return;
		}

	m_wndHeader.ModifyStyle(0, HDS_HIDDEN);
}

void CListView::AdjustLayout()
{
	// Preview
	if (m_PreviewAttribute>=0)
	{
		ASSERT(m_HasFolders);

		m_PreviewSize.cx = PREVIEWWIDTH;
		m_PreviewSize.cy = 128+m_SmallFontHeight;
	}
	else
	{
		m_PreviewSize.cx = m_PreviewSize.cy = 0;
	}

	// Header
	CRect rect;
	GetWindowRect(rect);

	WINDOWPOS wp;
	HDLAYOUT HdLayout;
	HdLayout.prc = &rect;
	HdLayout.pwpos = &wp;
	m_wndHeader.Layout(&HdLayout);

	wp.x = GetHeaderIndent();
	wp.y = 0;
	m_HeaderHeight = wp.cy;

	// Scroll area
	m_ScrollWidth = 0;

	for (UINT a=0; a<LFAttributeCount; a++)
		if ((INT)a!=m_PreviewAttribute)
			m_ScrollWidth += m_ContextViewSettings.ColumnWidth[a];

	// Folders
	m_Folders.m_ItemCount = 0;

	if (m_HasFolders && m_pFolderItems)
	{
		// Adjust layout with folders
		m_ScrollWidth += 2*(GUTTER+CARDPADDING-ITEMPADDING)+m_PreviewSize.cx;
		m_ScrollHeight = GUTTER;
		INT LastFolderItem = -1;

		for (UINT a=0; a<m_pFolderItems->m_ItemCount; a++)
		{
			// Folder
			LFItemDescriptor* pItemDescriptor = (*m_pFolderItems)[a];

			FolderData Data;
			Data.Rect.top = m_ScrollHeight;
			Data.Rect.bottom = Data.Rect.top+CARDPADDING;
			Data.Rect.left = GUTTER;
			Data.Rect.right = m_ScrollWidth-GUTTER;

			if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder)
			{
				VERIFY((Data.First=pItemDescriptor->AggregateFirst)>=0);
				VERIFY((LastFolderItem=Data.Last=pItemDescriptor->AggregateLast)>=0);

				if (pItemDescriptor->CoreAttributes.FileName[0])
					Data.Rect.bottom += m_LargeFontHeight+((m_PreviewAttribute>=0) ? CARDPADDING+PREVIEWITEMOFFSET : CARDPADDING/2+LFCATEGORYPADDING);

				Data.pItemDescriptor = pItemDescriptor;
			}
			else
			{
				Data.First = (UINT)(LastFolderItem+1);
				Data.Last = p_CookedFiles->m_ItemCount-1;

				Data.pItemDescriptor = NULL;

				a = (UINT)-2;
			}

			// Files
			CRect rectFile(Data.Rect.left+CARDPADDING-ITEMPADDING+m_PreviewSize.cx, Data.Rect.bottom-ITEMPADDING, m_ScrollWidth-GUTTER-CARDPADDING+ITEMPADDING, Data.Rect.bottom-ITEMPADDING+m_RowHeight+1);

			for (UINT b=(UINT)Data.First; b<=(UINT)Data.Last; b++)
			{
				FVItemData* pData = GetItemData(b);
				pData->Rect = rectFile;

				rectFile.OffsetRect(0, m_RowHeight);
			}

			Data.Rect.bottom += max(Data.pItemDescriptor ? m_PreviewSize.cy-PREVIEWITEMOFFSET : 0, m_RowHeight*(Data.Last-Data.First+1)-2*ITEMPADDING+1)+CARDPADDING;
			m_Folders.AddItem(Data);

			m_ScrollHeight = Data.Rect.bottom+GUTTER;
		}
	}
	else
	{
		// Adjust layout with folders
		m_ScrollWidth += BACKSTAGEBORDER;
		m_ScrollHeight = 1;

		if (p_CookedFiles)
		{
			CRect rectFile(BACKSTAGEBORDER, 1, m_ScrollWidth-1, m_RowHeight+2);

			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
			{
				FVItemData* pData = GetItemData(a);
				pData->Rect = rectFile;

				rectFile.OffsetRect(0, m_RowHeight);
			}

			m_ScrollHeight = rectFile.bottom;
		}
	}

	CFileView::AdjustLayout();

	// Header
	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

RECT CListView::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	// Find file name column
	for (UINT a=0; a<LFAttributeCount; a++)
	{
		const UINT Attr = m_ContextViewSettings.ColumnOrder[a];

		if (Attr==LFAttrFileName)
			break;

		if ((INT)Attr!=m_PreviewAttribute)
			rect.left += m_ContextViewSettings.ColumnWidth[Attr];
	}

	// Calculate rectangle
	rect.right = rect.left+m_ContextViewSettings.ColumnWidth[0]-3*ITEMPADDING;
	rect.left += m_IconSize+2*ITEMPADDING+GetColorDotWidth(Index)-2;

	return rect;
}

void CListView::DrawFolder(CDC& dc, Graphics& g, CRect& rect, INT Index, BOOL Themed)
{
	// Card
	DrawCardForeground(dc, g, rect, Themed);

	// Caption and preview
	LFItemDescriptor* pItemDescriptor = m_Folders[Index].pItemDescriptor;

	if (pItemDescriptor)
	{
		if (m_PreviewAttribute>=0)
			rect.left += 2;

		rect.DeflateRect(CARDPADDING, CARDPADDING);

		if (pItemDescriptor->CoreAttributes.FileName[0])
		{
			rect.top -= LFCATEGORYPADDING;

			DrawCategory(dc, rect, pItemDescriptor->CoreAttributes.FileName, NULL, Themed);
		}

		// Preview
		if (m_PreviewAttribute>=0)
		{
			rect.left -= 2;
			rect.top += m_LargeFontHeight+LFCATEGORYPADDING+CARDPADDING;

			DrawJumboIcon(dc, g, rect.TopLeft(), pItemDescriptor);

			// Description
			if (pItemDescriptor->Type & LFTypeHasDescription)
			{
				rect.right = rect.left+m_PreviewSize.cx-CARDPADDING;
				rect.top += 128+1;

				CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);

				dc.SetTextColor(Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW));
				dc.DrawText(pItemDescriptor->Description, -1, rect, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

				dc.SelectObject(pOldFont);
			}
		}
	}
	else
	{
		rect.DeflateRect(CARDPADDING, 0);
	}
}

void CListView::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	DrawItemBackground(dc, rectItem, Index, Themed);

	CRect rect(rectItem);
	rect.right = rect.left-3*ITEMPADDING;

	// Columns
	for (UINT a=0; a<LFAttributeCount; a++)
	{
		const UINT Attr = m_ContextViewSettings.ColumnOrder[a];

		if (m_ContextViewSettings.ColumnWidth[Attr] && ((INT)Attr!=m_PreviewAttribute))
		{
			rect.left = rect.right+SPACER;
			rect.right = rect.left+m_ContextViewSettings.ColumnWidth[Attr]-SPACER;

			if (pItemDescriptor->AttributeValues[Attr])
			{
				if (Attr==LFAttrFileName)
				{
					// Icon
					theApp.m_IconFactory.DrawSmallIcon(dc, CPoint(rect.left, (rect.top+rect.bottom-m_IconSize)/2), pItemDescriptor);

					rect.left += m_IconSize+2*ITEMPADDING;

					// Color
					DrawColorDots(dc, rect, pItemDescriptor, m_DefaultFontHeight);

					// Label
					if (IsEditing() && (Index==m_EditLabel))
						continue;
				}

				// Column
				if (theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeRating)
				{
					// Rating bitmap
					const UCHAR Rating = *((UCHAR*)pItemDescriptor->AttributeValues[Attr]);
					ASSERT(Rating<=LFMaxRating);

					CDC dcMem;
					dcMem.CreateCompatibleDC(&dc);

					HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(Attr==LFAttrPriority ? theApp.hPriorityBitmaps[Rating] : theApp.hRatingBitmaps[Rating]);

					dc.AlphaBlend(rect.left, (rect.top+rect.bottom-RATINGBITMAPHEIGHT)/2-1, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, &dcMem, 0, 0, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, BF);

					SelectObject(dcMem, hOldBitmap);
				}
				else
				{
					// Text column
					WCHAR tmpStr[256];

					switch (Attr)
					{
					case LFAttrFileName:
						wcsncpy_s(tmpStr, 256, GetLabel(pItemDescriptor), _TRUNCATE);
						break;

					case LFAttrFileFormat:
						wcscpy_s(tmpStr, 256, theApp.m_IconFactory.GetTypeName(pItemDescriptor->CoreAttributes.FileFormat));
						break;

					default:
						LFAttributeToString(pItemDescriptor, Attr, tmpStr, 256);
					}

					if (tmpStr[0])
						dc.DrawText(tmpStr, rect, (theApp.m_Attributes[Attr].TypeProperties.FormatRight ? DT_RIGHT : DT_LEFT) | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
				}
			}
		}
	}
}

void CListView::ScrollWindow(INT dx, INT dy, LPCRECT lpRect, LPCRECT lpClipRect)
{
	// Header
	if (IsWindow(m_wndHeader) && (dx!=0))
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		WINDOWPOS wp;
		HDLAYOUT HdLayout;
		HdLayout.prc = &rectWindow;
		HdLayout.pwpos = &wp;
		m_wndHeader.Layout(&HdLayout);

		wp.x = GetHeaderIndent();
		wp.y = 0;

		m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
	}

	if (IsCtrlThemed())
	{
		Invalidate();
	}
	else
	{
		CFileView::ScrollWindow(dx, dy, lpRect, lpClipRect);
	}
}

INT CListView::GetMaxAttributeWidth(UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);

	// Hidden columns
	if (!theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth)
		return 0;

	// Fixed width for rating bitmaps
	if (theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeRating)
		return RATINGBITMAPWIDTH+SPACER;

	// Preview
	if ((INT)Attr==m_PreviewAttribute)
		return PREVIEWWIDTH;

	// Calculate width
	INT Width = 0;

	if (p_CookedFiles)
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
		{
			INT cx = SPACER;

			if (Attr==LFAttrFileName)
			{
				cx += dc.GetTextExtent(GetLabel((*p_CookedFiles)[a])).cx;
			}
			else
			{
				WCHAR tmpStr[256];
				LFAttributeToString((*p_CookedFiles)[a], Attr, tmpStr, 256);

				cx += dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr)).cx;
			}

			if (cx>Width)
			{
				Width = cx;

				// Abort loop when MAXAUTOWIDTH has been reached or exceeded
				if (Width>=MAXAUTOWIDTH)
				{
					Width = MAXAUTOWIDTH;
					break;
				}
			}
		}
	
		dc.SelectObject(pOldFont);
	}

	// Icon
	if (Attr==LFAttrFileName)
		Width += m_IconSize+4*m_DefaultColorDots.GetIconSize()/3+2*ITEMPADDING;

	return max(Width, GetMinColumnWidth(Attr));
}

void CListView::AutosizeColumn(UINT Attr)
{
	ASSERT(Attr<LFAttributeCount);

	m_ContextViewSettings.ColumnWidth[Attr] = p_ContextViewSettings->ColumnWidth[Attr] = GetMaxAttributeWidth(Attr);
}

inline INT CListView::GetHeaderIndent() const
{
	return m_HasFolders ? GUTTER+CARDPADDING-ITEMPADDING-1 : BACKSTAGEBORDER-1;
}


BEGIN_MESSAGE_MAP(CListView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()

	ON_COMMAND_RANGE(IDM_LIST_TOGGLEATTRIBUTE, IDM_LIST_TOGGLEATTRIBUTE+LFAttributeCount-1, OnToggleAttribute)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_LIST_TOGGLEATTRIBUTE, IDM_LIST_TOGGLEATTRIBUTE+LFAttributeCount-1, OnUpdateToggleCommands)

	ON_COMMAND(IDM_LIST_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(IDM_LIST_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_LIST_CHOOSE, OnChooseDetails)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_LIST_AUTOSIZEALL, IDM_LIST_CHOOSE, OnUpdateDetailsCommands)

	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_BEGINTRACK, 1, OnBeginTrack)
	ON_NOTIFY(HDN_ENDDRAG, 1, OnEndDrag)
	ON_NOTIFY(HDN_ITEMCHANGING, 1, OnItemChanging)
	ON_NOTIFY(HDN_ITEMCLICK, 1, OnItemClick)
END_MESSAGE_MAP()

INT CListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Header
	if (!m_wndHeader.Create(this, 1))
		return -1;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		HDITEM HdItem;
		HdItem.mask = HDI_TEXT | HDI_FORMAT;
		HdItem.pszText = theApp.m_Attributes[a].Name;
		HdItem.fmt = HDF_STRING;
		m_wndHeader.InsertItem(a, &HdItem);
	}

	// Items
	m_IconSize = GetSystemMetrics(SM_CYSMICON);
	m_RowHeight = max(m_IconSize, m_DefaultFontHeight)+2*ITEMPADDING;

	return 0;
}

void CListView::OnDestroy()
{
	if (m_pFolderItems)
		LFFreeSearchResult(m_pFolderItems);

	CFileView::OnDestroy();
}

void CListView::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	Graphics g(dc);

	// Background
	const BOOL Themed = IsCtrlThemed();

	if (m_HasFolders)
	{
		DrawCardBackground(dc, g, CRect(rect.left, rect.top+m_HeaderHeight, rect.right, rect.bottom), Themed);
	}
	else
	{
		dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));
	}

	// Items
	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (!DrawNothing(dc, rect, Themed))
	{
		RECT rectIntersect;

		// Folders
		if (m_HasFolders)
			for (UINT a=0; a<m_Folders.m_ItemCount; a++)
			{
				CRect rect(m_Folders[a].Rect);
				rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+m_HeaderHeight);

				if (IntersectRect(&rectIntersect, rect, rectUpdate))
					DrawFolder(dc, g, rect, a, Themed);
			}

		// Items
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			FVItemData* pData = GetItemData(a);

			if (pData->Valid)
			{
				CRect rect(pData->Rect);
				rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+m_HeaderHeight);

				if (IntersectRect(&rectIntersect, rect, rectUpdate))
					DrawItem(dc, rect, a, Themed);
			}
		}
	}

	// Header
	if (Themed)
	{
		dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, 0xFFFFFF);

		Bitmap* pDivider = theApp.GetCachedResourceImage(IDB_DIVUP);

		g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2+GetScrollPos(SB_HORZ)+GUTTER+CARDPADDING-1, m_HeaderHeight-(INT)pDivider->GetHeight());
	}
	else
	{
		dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, GetSysColor(COLOR_3DFACE));
	}

	DrawWindowEdge(g, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CListView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd->GetSafeHwnd()==m_wndHeader)
	{
		OnDestroyEdit();

		// Add advertised attributes
		CMenu Menu;
		Menu.LoadMenu(IDM_LIST);

		CMenu* pPopup = Menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		for (INT a=LFAttributeCount-1; a>=0; a--)
			if (theApp.IsAttributeAdvertised(m_Context, a))
				pPopup->InsertMenu(3, MF_BYPOSITION | MF_STRING, IDM_LIST_TOGGLEATTRIBUTE+a, theApp.m_Attributes[a].Name);

		// Header item clicked
		CPoint pt(point);
		ScreenToClient(&pt);

		HDHITTESTINFO htt;
		htt.pt = pt;

		m_HeaderItemClicked = m_wndHeader.HitTest(&htt);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);

		return;
	}

	CFileView::OnContextMenu(pWnd, point);
}

void CListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	if (p_CookedFiles)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		INT Item = m_FocusItem;

		switch (nChar)
		{
		case VK_PRIOR:
			Item -= max(1, (rectClient.Height()-(INT)m_HeaderHeight)/m_RowHeight);
			break;

		case VK_NEXT:
			Item += max(1, (rectClient.Height()-(INT)m_HeaderHeight)/m_RowHeight);
			break;

		case VK_UP:
			Item--;
			break;

		case VK_DOWN:
			Item++;
			break;

		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
				Item = 0;

			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
				Item = ((INT)p_CookedFiles->m_ItemCount)-1;

			break;
		}

		if (Item<0)
			Item = 0;

		if (Item>=(INT)p_CookedFiles->m_ItemCount)
			Item = ((INT)p_CookedFiles->m_ItemCount)-1;

		if (Item!=m_FocusItem)
		{
			m_ShowFocusRect = TRUE;
			SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);

			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			OnMouseMove(0, pt);
		}
	}
}


// Toggle attributes

void CListView::OnToggleAttribute(UINT nID)
{
	const UINT Attr = nID-IDM_LIST_TOGGLEATTRIBUTE;
	ASSERT(Attr<LFAttributeCount);

	p_ContextViewSettings->ColumnWidth[Attr] = p_ContextViewSettings->ColumnWidth[Attr] ? 0 : theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth;

	theApp.UpdateViewSettings(m_Context);
}

void CListView::OnUpdateToggleCommands(CCmdUI* pCmdUI)
{
	const UINT Attr = pCmdUI->m_nID-IDM_LIST_TOGGLEATTRIBUTE;
	ASSERT(Attr<LFAttributeCount);

	pCmdUI->SetCheck(m_ContextViewSettings.ColumnWidth[Attr]);
	pCmdUI->Enable(!theApp.m_Attributes[Attr].AttrProperties.AlwaysShow && ((INT)Attr!=m_PreviewAttribute));
}


// Autosize

void CListView::OnAutosizeAll()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		if (m_ContextViewSettings.ColumnWidth[a] && ((INT)a!=m_PreviewAttribute))
			AutosizeColumn(a);

	AdjustHeader();
	AdjustLayout();
}

void CListView::OnAutosize()
{
	if (m_HeaderItemClicked>=0)
	{
		AutosizeColumn(m_HeaderItemClicked);

		AdjustHeader();
		AdjustLayout();
	}
}

void CListView::OnChooseDetails()
{
	ChooseDetailsDlg dlg(m_Context, this);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewSettings(m_Context);
}

void CListView::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(pCmdUI->m_nID==IDM_LIST_AUTOSIZE ? (m_HeaderItemClicked>=0) && (m_HeaderItemClicked!=m_PreviewAttribute) : TRUE);
}


// Header commands

void CListView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	OnDestroyEdit();

	// Do not drag invisible columns, hidden attributes or the preview attribute
	const UINT Attr = pHdr->iItem;

	*pResult =
		(m_ContextViewSettings.ColumnWidth[Attr]==0) ||							// Currently invisible column
		(theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth==0) ||		// Hidden attribute
		((INT)Attr==m_PreviewAttribute);										// Preview attribute of this list
}

void CListView::OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_ORDER)
	{
		const INT Attr = pHdr->iItem;

		if (pHdr->pitem->iOrder==-1)
		{
			// Attribute dragged out of header
			p_ContextViewSettings->ColumnWidth[Attr] = 0;
		}
		else
		{
			// Attribute dropped - get real order array with possible preview column at position 0
			INT OrderArray[LFAttributeCount];
			m_wndHeader.GetOrderArray(OrderArray, LFAttributeCount);

			// Copy order array to context view settings, and insert dragged column
			UINT ReadIndex = 0;
			UINT WriteIndex = 0;

			for (UINT a=0; a<LFAttributeCount; a++)
				if (pHdr->pitem->iOrder==(INT)a)
				{
					p_ContextViewSettings->ColumnOrder[WriteIndex++] = Attr;
				}
				else
				{
					if (OrderArray[ReadIndex]==Attr)
						ReadIndex++;

					p_ContextViewSettings->ColumnOrder[WriteIndex++] = OrderArray[ReadIndex++];
				}
		}

		theApp.UpdateViewSettings(m_Context, LFViewList);

		*pResult = TRUE;
	}
}

void CListView::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
	{
		OnDestroyEdit();

		// Do not track rating columns, invisible columns, hidden attributes or the preview attribute
		const UINT Attr = pHdr->iItem;

		*pResult =
			(theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeRating) ||		// Fixed width
			(m_ContextViewSettings.ColumnWidth[Attr]==0) ||							// Currently invisible column
			(theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth==0) ||		// Hidden attribute
			((INT)Attr==m_PreviewAttribute);										// Preview attribute of this list
	}
}

void CListView::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (!m_IgnoreHeaderItemChange && (pHdr->pitem->mask & HDI_WIDTH))
	{
		const UINT Attr = pHdr->iItem;

		// Guarantee GetMinColumnWidth(), or hide column
		INT Width = 0;
		if (theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth)
			Width = (pHdr->pitem->cxy<GetMinColumnWidth(Attr)) ? theApp.m_Attributes[Attr].AttrProperties.AlwaysShow ? GetMinColumnWidth(Attr) : 0 : pHdr->pitem->cxy;

		if ((Width!=m_ContextViewSettings.ColumnWidth[Attr]) || (Width!=pHdr->pitem->cxy))
		{
			p_ContextViewSettings->ColumnWidth[Attr] = Width;

			theApp.UpdateViewSettings(m_Context, LFViewList);
		}

		*pResult = TRUE;
	}
}

void CListView::OnItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	const UINT Attr = pHdr->iItem;

	// Remain in list view when possible!
	theApp.SetContextSort(m_Context, Attr, p_ContextViewSettings->SortBy==Attr ? !p_ContextViewSettings->Descending : theApp.m_Attributes[Attr].TypeProperties.DefaultDescending, FALSE);

	*pResult = FALSE;
}
