
// CGridView.cpp: Implementierung der Klasse CGridView
//

#include "stdafx.h"
#include "CGridView.h"
#include "liquidFOLDERS.h"


// CGridView
//

#define GetItemData(Index)     ((GridItemData*)(m_pItemData+(Index)*m_DataSize))

CGridView::CGridView(SIZE_T DataSize, UINT Flags)
	: CFileView(DataSize, Flags | FF_ENABLESCROLLING | FF_ENABLEHOVER | FF_ENABLETOOLTIPS | FF_ENABLEFOLDERTOOLTIPS | FF_ENABLESHIFTSELECTION)
{
}

void CGridView::Arrange(CSize szItem, INT Padding, CSize szGutter, BOOL FullWidth)
{
	ResetItemCategories();

	if (!p_CookedFiles)
	{
		m_ScrollWidth = m_ScrollHeight = 0;
		return;
	}

	CRect rectWindow;
	GetWindowRect(rectWindow);

	if (!rectWindow.Width())
		return;

	ENSURE((szItem.cx+=2*Padding)>0);
	ENSURE((szItem.cy+=2*Padding)>0);

	m_RowHeight = szItem.cy+szGutter.cy;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = BACKSTAGEBORDER;
	INT y = m_HeaderHeight ? 1 : BACKSTAGEBORDER;

	INT Category = -1;

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		if (p_CookedFiles->m_HasCategories)
			if ((INT)(*p_CookedFiles)[a]->CategoryID!=Category)
			{
				if (x>BACKSTAGEBORDER)
				{
					Column = 0;
					Row++;

					x = BACKSTAGEBORDER;
					y += szItem.cy+szGutter.cy;
				}

				if (y>BACKSTAGEBORDER)
					y += 8;

				Category = (*p_CookedFiles)[a]->CategoryID;

				const LPRECT lpRect = &m_Categories[Category];
				lpRect->left = lpRect->right = x;
				lpRect->top = y;
				lpRect->bottom = lpRect->top+2*LFCATEGORYPADDING+m_LargeFontHeight;

				if (theApp.m_ItemCategories[Category].Hint[0]!=L'\0')
					lpRect->bottom += m_DefaultFontHeight;

				y = lpRect->bottom+4;
			}

		GridItemData* pData = GetItemData(a);
		pData->Column = Column++;
		pData->Row = Row;
		pData->Hdr.Rect.left = x;
		pData->Hdr.Rect.top = y;
		pData->Hdr.Rect.right = x+szItem.cx;
		pData->Hdr.Rect.bottom = y+szItem.cy;

		if ((x+=szItem.cx+szGutter.cx)>m_ScrollWidth)
			m_ScrollWidth = x-1;

		if (y+szItem.cy+szGutter.cy-1>m_ScrollHeight)
			if (((m_ScrollHeight=y+szItem.cy+max(szGutter.cy, 0))>rectWindow.Height()) && !HasScrollbars)
			{
				HasScrollbars = TRUE;
				rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL);

				goto Restart;
			}

		if ((x+szItem.cx+szGutter.cx>rectWindow.Width()) || FullWidth)
		{
			if (FullWidth)
				pData->Hdr.Rect.right = rectWindow.Width()-szGutter.cx;

			Column = 0;
			Row++;

			x = BACKSTAGEBORDER;
			y += szItem.cy+szGutter.cy;
		}
	}

	// Adjust categories to calculated width
	for (UINT a=0; a<LFItemCategoryCount; a++)
		if (m_Categories[a].right)
			m_Categories[a].right = max(m_ScrollWidth, rectWindow.Width())-2;

	CFileView::AdjustLayout();
}

void CGridView::DrawJumboIcon(CDC& dc, const CRect& rect, LFItemDescriptor* pItemDescriptor)
{
	if (pItemDescriptor->IconID)
	{
		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && theApp.m_Attributes[m_ContextViewSettings.SortBy].AttrProperties.ShowRepresentativeThumbnail)
			if (theApp.m_ThumbnailCache.DrawRepresentativeThumbnail(dc, rect, p_RawFiles, pItemDescriptor->FirstAggregate, pItemDescriptor->LastAggregate))
				return;

		const INT YOffset = (pItemDescriptor->IconID==IDI_FLD_PLACEHOLDER) ? 1 : 0;
		theApp.m_CoreImageListJumbo.DrawEx(&dc, pItemDescriptor->IconID-1, CPoint((rect.right+rect.left-128)/2+YOffset, (rect.top+rect.bottom-128)/2), CSize(128, 128), CLR_NONE, 0xFFFFFF, ((pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (pItemDescriptor->Type & LFTypeBadgeMask));
	}
	else
	{
		ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

		if (!theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rect, pItemDescriptor))
			theApp.m_FileFormats.DrawJumboIcon(dc, rect, pItemDescriptor->CoreAttributes.FileFormat, pItemDescriptor->Type & LFTypeGhosted);
	}
}


BEGIN_MESSAGE_MAP(CGridView, CFileView)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CGridView::OnPaint()
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

	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (!DrawNothing(dc, rect, Themed))
	{
		RECT rectIntersect;

		// Items
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			GridItemData* pData = GetItemData(a);
			if (pData->Hdr.Valid)
			{
				CRect rect(pData->Hdr.Rect);
				rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);
				if (IntersectRect(&rectIntersect, rect, rectUpdate))
				{
					DrawItemBackground(dc, rect, a, Themed);
					DrawItem(dc, rect, a, Themed);
					DrawItemForeground(dc, rect, a, Themed);
				}
			}
		}

		// Categories
		if (p_CookedFiles->m_HasCategories)
			for (UINT a=0; a<LFItemCategoryCount; a++)
			{
				CRect rect(m_Categories[a]);
				rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

				if (IntersectRect(&rectIntersect, rect, rectUpdate))
					DrawCategory(dc, rect, theApp.m_ItemCategories[a].Caption, theApp.m_ItemCategories[a].Hint, Themed);
			}
	}

	DrawWindowEdge(dc, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CGridView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	if (p_CookedFiles)
	{
		CRect rect;
		GetClientRect(rect);

		INT Item = m_FocusItem;
		const INT Column = (Item==-1) ? 0 : GetItemData(Item)->Column;
		const INT Row = (Item==-1) ? 0 : GetItemData(Item)->Row;
		const INT Top = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.top;
		const INT Bottom = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.bottom;
		INT TmpRow = -1;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				const GridItemData* pData = GetItemData(a);
				if ((pData->Row==Row) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_RIGHT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				const GridItemData* pData = GetItemData(a);
				if ((pData->Row==Row) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}

			}

			break;

		case VK_UP:
			for (INT a=Item-1; a>=0; a--)
			{
				const GridItemData* pData = GetItemData(a);
				if ((pData->Row<Row) && pData->Hdr.Valid)
				{
					Item = a;

					if (pData->Column<=Column)
						break;
				}

				if (pData->Row<Row-1)
					break;
			}

			break;

		case VK_PRIOR:
			for (INT a=Item-1; a>=0; a--)
			{
				const GridItemData* pData = GetItemData(a);
				if ((pData->Row<=Row) && (pData->Column<=Column) && pData->Hdr.Valid)
				{
					Item = a;

					if (pData->Hdr.Rect.top<=Bottom-rect.Height()+(INT)m_HeaderHeight)
						break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				const GridItemData* pData = GetItemData(a);
				if ((pData->Row>Row) && pData->Hdr.Valid)
				{
					Item = a;

					if (pData->Column>=Column)
						break;
				}

				if (pData->Row>Row+1)
					break;
			}

			break;

		case VK_NEXT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				const GridItemData* pData = GetItemData(a);
				if (pData->Row!=GetItemData(a-1)->Row)
					if (pData->Hdr.Rect.bottom>=Top+rect.Height()-(INT)m_HeaderHeight)
						if (TmpRow==-1)
						{
							TmpRow = pData->Row;
						}
						else
						{
							if (pData->Row>TmpRow)
								break;
						}

				if (((TmpRow==-1) || (pData->Column<=Column)) && pData->Hdr.Valid)
					Item = a;
			}

			break;

		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;
						break;
					}
			}
			else
				for (INT a=Item-1; a>=0; a--)
				{
					const GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
						if (pData->Row==Row)
						{
							Item = a;
						}
						else
						{
							break;
						}
				}

			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=(INT)p_CookedFiles->m_ItemCount-1; a>=0; a--)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;
						break;
					}
			}
			else
				for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
				{
					const GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
						if (pData->Row==Row)
						{
							Item = a;
						}
						else
						{
							break;
						}
				}

			break;
		}

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
