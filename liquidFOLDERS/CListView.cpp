
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"
#include "CListView.h"
#include "liquidFOLDERS.h"


// CListView
//

#define GetItemData(Index)     ((ListItemData*)CFileView::GetItemData(Index))
#define MAXAUTOWIDTH           400
#define PREVIEWITEMOFFSET      2
#define PREVIEWWIDTH           128+CARDPADDING
#define SPACER                 (4*ITEMCELLPADDING+1)

CListView::CListView()
	: CFileView(FRONTSTAGE_CARDBACKGROUND | FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FRONTSTAGE_ENABLELABELEDIT | FF_ENABLEFOLDERTOOLTIPS | FF_ENABLETOOLTIPICONS, sizeof(ListItemData))
{
	m_pFolderItems = NULL;
	m_HasFolders = FALSE;
}

void CListView::SetViewSettings(BOOL UpdateSearchResultPending)
{
	// Copy context view settings early for header
	m_ContextViewSettings = *p_ContextViewSettings;

	UpdateHeader();

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

	// Switch to alternate sort attribute if neccessary
	while (!theApp.IsAttributeSortable(m_Context, m_ContextViewSettings.SortBy, m_SubfolderAttribute) || ((INT)m_ContextViewSettings.SortBy==m_SubfolderAttribute))
	{
		m_ContextViewSettings.SortBy = (UINT)theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.AlternateSort;
		m_ContextViewSettings.SortDescending = theApp.IsAttributeSortDescending(m_Context, m_ContextViewSettings.SortBy);
	}

	// Group search result
	if (m_pFolderItems)
		LFFreeSearchResult(m_pFolderItems);

	if (p_RawFiles)
	{
		ValidateAllItems();

		// Attribute and icon
		const UINT Attr = m_ContextViewSettings.SortBy;
		const UINT Icon = theApp.GetAttributeIcon(Attr, pCookedFiles->m_Context);

		// Folders
		m_HasFolders = ((INT)Attr!=m_SubfolderAttribute) && (Icon || theApp.IsAttributeBucket(Attr));

		if (LFGetSubfolderAttribute(pFilter)==Attr)
			m_HasFolders = FALSE;

		// Preview attribute
		m_PreviewAttribute = m_HasFolders &&
			!theApp.ShowRepresentativeThumbnail(m_SubfolderAttribute, m_Context) &&
			(Icon || theApp.ShowRepresentativeThumbnail(Attr, pCookedFiles->m_Context)) ? Attr : -1;

		// Prepare search result
		if (m_HasFolders)
		{
			m_pFolderItems = LFGroupSearchResult(p_RawFiles, Attr, m_ContextViewSettings.SortDescending, TRUE, pFilter);
		}
		else
		{
			m_pFolderItems = NULL;

			LFSortSearchResult(p_RawFiles, Attr, m_ContextViewSettings.SortDescending);
		}
	}
	else
	{
		m_pFolderItems = NULL;
		m_HasFolders = FALSE;
		m_PreviewAttribute = -1;
	}

	// Adjust background style
	if (m_HasFolders)
	{
		m_Flags |= FRONTSTAGE_CARDBACKGROUND;
	}
	else
	{
		m_Flags &= ~FRONTSTAGE_CARDBACKGROUND;
	}

	UpdateHeader();
}

INT CListView::GetHeaderIndent() const
{
	return m_HasFolders ? BACKSTAGEBORDER+CARDPADDING-ITEMCELLPADDING-1 : BACKSTAGEBORDER-1;
}

void CListView::GetHeaderContextMenu(CMenu& Menu, INT HeaderItem)
{
	OnDestroyEdit();

	// Add advertised attributes
	Menu.LoadMenu(IDM_LIST);

	for (INT a=LFAttributeCount-1; a>=0; a--)
		if (theApp.IsAttributeAdvertised(m_Context, a) && theApp.m_Attributes[a].TypeProperties.DefaultColumnWidth)
			Menu.InsertMenu(3, MF_BYPOSITION | MF_STRING, IDM_LIST_TOGGLEATTRIBUTE+a, theApp.GetAttributeName(a, m_Context));

	m_HeaderItemClicked = HeaderItem;
}

BOOL CListView::AllowHeaderColumnDrag(UINT Attr) const
{
	return
		(m_ContextViewSettings.ColumnWidth[Attr]!=0) &&							// Visible column
		(theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth!=0) &&		// Visible attribute
		((INT)Attr!=m_PreviewAttribute);										// Not preview attribute
}

BOOL CListView::AllowHeaderColumnTrack(UINT Attr) const
{
	return
		AllowHeaderColumnDrag(Attr) &&
		(theApp.m_Attributes[Attr].AttrProperties.Type!=LFTypeRating);			// Variable width
}

void CListView::UpdateHeaderColumnOrder(UINT Attr, INT Position)
{
	CFileView::UpdateHeaderColumnOrder(Attr, Position, p_ContextViewSettings->ColumnOrder, p_ContextViewSettings->ColumnWidth);

	theApp.UpdateViewSettings(m_Context, LFViewList);
}

void CListView::UpdateHeaderColumnWidth(UINT Attr, INT Width)
{
	if (!theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth)
	{
		Width = 0;
	}
	else
	{
		if (Width<GetMinColumnWidth(Attr))
			Width = theApp.IsAttributeAlwaysVisible(Attr) ? GetMinColumnWidth(Attr) : 0;
	}

	if (Width!=p_ContextViewSettings->ColumnWidth[Attr])
	{
		p_ContextViewSettings->ColumnWidth[Attr] = Width;

		theApp.UpdateViewSettings(m_Context, LFViewList);
	}
}

void CListView::UpdateHeaderColumn(UINT Attr, HDITEM& HeaderItem) const
{
	HeaderItem.mask = HDI_WIDTH | HDI_FORMAT | HDI_TEXT;
	HeaderItem.cxy = ((INT)Attr==m_PreviewAttribute) ? PREVIEWWIDTH : ((INT)Attr==m_SubfolderAttribute) ? 0 : p_ContextViewSettings->ColumnWidth[Attr];
	HeaderItem.fmt = HDF_STRING | (theApp.IsAttributeFormatRight(Attr) ? HDF_RIGHT : HDF_LEFT);
	HeaderItem.pszText = (LPWSTR)theApp.GetAttributeName(Attr, m_Context);

	if (m_ContextViewSettings.SortBy==Attr)
		HeaderItem.fmt |= m_ContextViewSettings.SortDescending ? HDF_SORTDOWN : HDF_SORTUP;

	if (theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth && HeaderItem.cxy)
	{
		if (theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeRating)
		{
			HeaderItem.cxy = RATINGBITMAPWIDTH+SPACER;
		}
		else
		{
			if (HeaderItem.cxy<GetMinColumnWidth(Attr))
				HeaderItem.cxy = GetMinColumnWidth(Attr);
		}
	}
	else
	{
		HeaderItem.cxy = 0;
	}
}

void CListView::HeaderColumnClicked(UINT Attr)
{
	if (theApp.IsAttributeSortable(m_Context, Attr, m_SubfolderAttribute))
		theApp.SetContextSort(m_Context, Attr, m_ContextViewSettings.SortBy==Attr ? !m_ContextViewSettings.SortDescending : theApp.IsAttributeSortDescending(m_Context, Attr), FALSE);
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
		m_ScrollWidth += 2*(BACKSTAGEBORDER+CARDPADDING-ITEMCELLPADDING)+m_PreviewSize.cx;
		m_ScrollHeight = BACKSTAGEBORDER;
		INT LastFolderItem = -1;
		INT Row = 0;

		for (UINT a=0; a<m_pFolderItems->m_ItemCount; a++)
		{
			// Folder
			LFItemDescriptor* pItemDescriptor = (*m_pFolderItems)[a];

			FolderData Data;
			Data.Rect.bottom = (Data.Rect.top=m_ScrollHeight)+CARDPADDING;
			Data.Rect.left = BACKSTAGEBORDER;
			Data.Rect.right = m_ScrollWidth-BACKSTAGEBORDER;

			if (LFIsFolder(pItemDescriptor))
			{
				VERIFY((Data.First=pItemDescriptor->AggregateFirst)>=0);
				VERIFY((LastFolderItem=Data.Last=pItemDescriptor->AggregateLast)>=0);

				if (pItemDescriptor->CoreAttributes.FileName[0])
				{
					Data.Rect.bottom += m_LargeFontHeight+((m_PreviewAttribute>=0) ? CARDPADDING+PREVIEWITEMOFFSET : CARDPADDING/2+LFCATEGORYPADDING);

					if (pItemDescriptor->FolderMainCaptionCount)
						Data.Rect.bottom += m_DefaultFontHeight;
				}

				Data.pItemDescriptor = pItemDescriptor;
			}
			else
			{
				Data.First = (UINT)(LastFolderItem+1);
				Data.Last = m_ItemCount-1;

				Data.pItemDescriptor = NULL;

				a = (UINT)-2;
			}

			// Files
			CRect rectFile(Data.Rect.left+CARDPADDING-ITEMCELLPADDING+m_PreviewSize.cx, Data.Rect.bottom-ITEMCELLPADDING, m_ScrollWidth-BACKSTAGEBORDER-CARDPADDING+ITEMCELLPADDING, Data.Rect.bottom-ITEMCELLPADDING+m_ItemHeight);
			for (UINT b=(UINT)Data.First; b<=(UINT)Data.Last; b++)
			{
				ListItemData* pData = GetItemData(b);
				pData->Hdr.Rect = rectFile;
				pData->Hdr.Row = Row++;
				pData->DrawTrailingSeparator = (b<(UINT)Data.Last);

				rectFile.OffsetRect(0, m_szScrollStep.cy);
			}

			Data.Rect.bottom += max(Data.pItemDescriptor ? m_PreviewSize.cy-PREVIEWITEMOFFSET : 0, m_szScrollStep.cy*(Data.Last-Data.First+1)-2*ITEMCELLPADDING+1)+CARDPADDING;
			m_Folders.AddItem(Data);

			m_ScrollHeight = Data.Rect.bottom+BACKSTAGEBORDER;
		}
	}
	else
	{
		// Adjust layout without folders
		m_ScrollWidth += BACKSTAGEBORDER;
		m_ScrollHeight = 1;

		if (p_CookedFiles)
		{
			CRect rectFile(BACKSTAGEBORDER, 1, m_ScrollWidth-1, m_ItemHeight+1);

			for (INT a=0; a<m_ItemCount; a++)
			{
				ListItemData* pData = GetItemData(a);
				pData->Hdr.Rect = rectFile;
				pData->DrawTrailingSeparator = (a<m_ItemCount-1);

				rectFile.OffsetRect(0, m_szScrollStep.cy);
			}

			m_ScrollHeight = rectFile.bottom;
		}
	}

	CFileView::AdjustLayout();
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
	rect.right = rect.left+m_ContextViewSettings.ColumnWidth[0]-3*ITEMCELLPADDING;
	rect.left += m_IconSize+2*ITEMCELLPADDING+GetColorDotWidth(Index)-2;

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

			if (pItemDescriptor->FolderMainCaptionCount)
			{
				WCHAR Caption[256];
				wcsncpy_s(Caption, 256, pItemDescriptor->CoreAttributes.FileName, pItemDescriptor->FolderMainCaptionCount);
			
				DrawCategory(dc, rect, Caption, &pItemDescriptor->CoreAttributes.FileName[pItemDescriptor->FolderSubcaptionStart], Themed);
			}
			else
			{
				DrawCategory(dc, rect, pItemDescriptor->CoreAttributes.FileName, NULL, Themed);
			}
		}

		// Preview
		if (m_PreviewAttribute>=0)
		{
			rect.left -= 2;
			rect.top += m_LargeFontHeight+LFCATEGORYPADDING+CARDPADDING;

			if (pItemDescriptor->FolderMainCaptionCount)
				rect.top += m_DefaultFontHeight;

			DrawJumboIcon(dc, g, rect.TopLeft(), pItemDescriptor);

			// Description
			rect.right = rect.left+m_PreviewSize.cx-CARDPADDING;
			rect.top += 128+1;

			CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);

			SetLightTextColor(dc, pItemDescriptor, Themed);
			dc.DrawText(pItemDescriptor->Description, -1, rect, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

			dc.SelectObject(pOldFont);
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
	ListItemData* pData = GetItemData(Index);

	// Trailing separator
	if (pData->DrawTrailingSeparator && Themed)
		dc.FillSolidRect(rectItem->left+ITEMCELLPADDING, rectItem->bottom-1, rectItem->right-rectItem->left-2*ITEMCELLPADDING, 1, 0xF8F6F6);

	// Background
	DrawItemBackground(dc, rectItem, Index, Themed);

	CRect rect(rectItem);
	rect.right = rect.left-3*ITEMCELLPADDING;

	// Columns
	COLORREF TextColors[2] = { SetDarkTextColor(dc, pItemDescriptor, Themed), dc.GetTextColor() };

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		const UINT Attr = m_ContextViewSettings.ColumnOrder[a];

		if (m_ContextViewSettings.ColumnWidth[Attr] && ((INT)Attr!=m_PreviewAttribute))
		{
			rect.right = (rect.left=rect.right+SPACER)+m_ContextViewSettings.ColumnWidth[Attr]-SPACER;

			if (pItemDescriptor->AttributeValues[Attr])
			{
				if (Attr==LFAttrFileName)
				{
					// Icon
					theApp.m_IconFactory.DrawSmallIcon(dc, CPoint(rect.left, (rect.top+rect.bottom-m_IconSize)/2), pItemDescriptor);

					rect.left += m_IconSize+2*ITEMCELLPADDING;

					// Color
					DrawColorDots(dc, rect, pItemDescriptor, m_DefaultFontHeight);

					// Label
					if (IsEditing() && (Index==m_EditItem))
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
						wcsncpy_s(tmpStr, 256, GetItemLabel(pItemDescriptor), _TRUNCATE);
						break;

					case LFAttrFileFormat:
						wcscpy_s(tmpStr, 256, theApp.m_IconFactory.GetTypeName(pItemDescriptor->CoreAttributes.FileFormat));
						break;

					default:
						LFAttributeToString(pItemDescriptor, Attr, tmpStr, 256);
					}

					if (tmpStr[0])
					{
						dc.SetTextColor(TextColors[Attr==LFAttrFileName ? 0 : 1]);
						dc.DrawText(tmpStr, rect, (theApp.IsAttributeFormatRight(Attr) ? DT_RIGHT : DT_LEFT) | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
					}
				}
			}
		}
	}
}

void CListView::DrawStage(CDC& dc, Graphics& g, const CRect& /*rect*/, const CRect& rectUpdate, BOOL Themed)
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
	for (INT a=0; a<m_ItemCount; a++)
	{
		ListItemData* pData = GetItemData(a);

		if (pData->Hdr.Valid)
		{
			CRect rect(pData->Hdr.Rect);
			rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+m_HeaderHeight);

			if (IntersectRect(&rectIntersect, rect, rectUpdate))
				DrawItem(dc, rect, a, Themed);
		}
	}
}

void CListView::ScrollWindow(INT dx, INT dy, LPCRECT lpRect, LPCRECT lpClipRect)
{

	CFileView::ScrollWindow(dx, dy, lpRect, lpClipRect);
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

		for (INT a=0; a<m_ItemCount; a++)
		{
			INT cx = SPACER;

			if (Attr==LFAttrFileName)
			{
				cx += dc.GetTextExtent(GetItemLabel((*p_CookedFiles)[a])).cx;
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
		Width += m_IconSize+4*m_DefaultColorDots.GetIconSize()/3+2*ITEMCELLPADDING;

	return max(Width, GetMinColumnWidth(Attr));
}

void CListView::AutosizeColumn(UINT Attr)
{
	ASSERT(Attr<LFAttributeCount);

	m_ContextViewSettings.ColumnWidth[Attr] = p_ContextViewSettings->ColumnWidth[Attr] = GetMaxAttributeWidth(Attr);
}


BEGIN_MESSAGE_MAP(CListView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()

	ON_COMMAND_RANGE(IDM_LIST_TOGGLEATTRIBUTE, IDM_LIST_TOGGLEATTRIBUTE+LFAttributeCount-1, OnToggleAttribute)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_LIST_TOGGLEATTRIBUTE, IDM_LIST_TOGGLEATTRIBUTE+LFAttributeCount-1, OnUpdateToggleCommands)

	ON_COMMAND(IDM_LIST_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(IDM_LIST_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_LIST_CHOOSE, OnChooseDetails)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_LIST_AUTOSIZEALL, IDM_LIST_CHOOSE, OnUpdateDetailsCommands)
END_MESSAGE_MAP()

INT CListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Header
	AddHeaderColumns();

	// Items
	SetItemHeight(GetSystemMetrics(SM_CYSMICON), 1, ITEMCELLPADDING);

	return 0;
}

void CListView::OnDestroy()
{
	if (m_pFolderItems)
		LFFreeSearchResult(m_pFolderItems);

	CFileView::OnDestroy();
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
	pCmdUI->Enable(!theApp.IsAttributeAlwaysVisible(Attr) && ((INT)Attr!=m_PreviewAttribute));
}


// Autosize

void CListView::OnAutosizeAll()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		if (m_ContextViewSettings.ColumnWidth[a] && ((INT)a!=m_PreviewAttribute))
			AutosizeColumn(a);

	UpdateHeader();
	AdjustLayout();
}

void CListView::OnAutosize()
{
	if (m_HeaderItemClicked>=0)
	{
		AutosizeColumn(m_HeaderItemClicked);

		UpdateHeader();
		AdjustLayout();
	}
}

void CListView::OnChooseDetails()
{
	if (ChooseDetailsDlg(this, m_Context).DoModal()==IDOK)
		theApp.UpdateViewSettings(m_Context);
}

void CListView::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(pCmdUI->m_nID==IDM_LIST_AUTOSIZE ? (m_HeaderItemClicked>=0) && (m_HeaderItemClicked!=m_PreviewAttribute) : TRUE);
}
