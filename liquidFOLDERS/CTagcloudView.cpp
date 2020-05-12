
// CTagcloudView.cpp: Implementierung der Klasse CTagcloudView
//

#include "stdafx.h"
#include "CTagcloudView.h"
#include "liquidFOLDERS.h"


// CTagcloudView
//

#define DEFAULTFONT     3
#define TEXTFORMAT      DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE
#define GUTTER          3
#define MARGIN          BACKSTAGEBORDER
#define PADDINGX        5
#define PADDINGY        4


CTagcloudView::CTagcloudView()
	: CFileView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FF_ENABLEFOLDERTOOLTIPS | FRONTSTAGE_ENABLELABELEDIT | FF_ENABLETOOLTIPICONS, sizeof(TagcloudItemData))
{
}

void CTagcloudView::SetViewSettings(BOOL UpdateSearchResultPending)
{
	UINT Changes = 0;

	if (m_GlobalViewSettings.TagcloudUseSize!=p_GlobalViewSettings->TagcloudUseSize)
		Changes = 1;

	if ((m_GlobalViewSettings.TagcloudSort!=p_GlobalViewSettings->TagcloudSort) || (m_GlobalViewSettings.TagcloudShowRare!=p_GlobalViewSettings->TagcloudShowRare))
		Changes = 2;

	// Commit settings
	if (p_CookedFiles && !UpdateSearchResultPending)
	{
		// Copy global view settings early for sorting
		m_GlobalViewSettings = *p_GlobalViewSettings;

		switch (Changes)
		{
		case 2:
			SetSearchResult(p_Filter, p_RawFiles, p_CookedFiles, NULL);
			FinishUpdate();
			break;

		case 1:
			AdjustLayout();
			break;

		case 0:
			Invalidate();
			break;
		}
	}
}

void CTagcloudView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	if (p_CookedFiles)
	{
		// Sort cooked files
		LFSortSearchResult(p_CookedFiles, (m_GlobalViewSettings.TagcloudSort==0) ? m_ContextViewSettings.SortBy : LFAttrFileCount,
			(m_GlobalViewSettings.TagcloudSort!=0) || theApp.IsAttributeSortDescending(m_Context, m_ContextViewSettings.SortBy));

		INT Minimum = -1;
		INT Maximum = -1;

		// Find items
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];
			TagcloudItemData* pData = GetTagcloudItemData(a);

			if ((pData->Hdr.Valid=LFIsFolder(pItemDescriptor))==TRUE)
			{
				pData->Cnt = pItemDescriptor->AggregateCount;

				Minimum = (Minimum==-1) ? pData->Cnt : min(Minimum, pData->Cnt);
				Maximum = (Maximum==-1) ? pData->Cnt : max(Maximum, pData->Cnt);
			}
		}

		// Calculate display properties
		const INT Delta = Maximum-Minimum+1;

		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TagcloudItemData* pData = GetTagcloudItemData(a);
			if (pData->Hdr.Valid)
				if (!m_GlobalViewSettings.TagcloudShowRare && (Delta>1) && (pData->Cnt==Minimum))
				{
					// Omit rare tag
					ZeroMemory(pData, sizeof(TagcloudItemData));
				}
				else
					if (Delta==1)
					{
						// Fixed minimum
						pData->FontSize = 19;
						pData->Alpha = 255;
						pData->Color = 0xFF0000;
					}
					else
					{
						pData->FontSize = (20*(pData->Cnt-Minimum))/Delta;
						pData->Alpha = 56+(200*(pData->Cnt-Minimum))/Delta;

						if (pData->Alpha<128)
						{
							pData->Color = 0xFF0000 | ((128-pData->Alpha)<<9);
						}
						else
							if (pData->Alpha<192)
							{
								pData->Color = (0xFF0000-(pData->Alpha-128)*0x020000) | ((pData->Alpha-128)*2);
							}
							else
							{
								pData->Color = (0x800000-(pData->Alpha-192)*0x020000) | (0x000080+(pData->Alpha-192)*2);
							}
					}
		}
	}
}


// Menus

BOOL CTagcloudView::GetContextMenu(CMenu& Menu, INT Index)
{
	if (Index==-1)
		Menu.LoadMenu(IDM_TAGCLOUD);

	return CFileView::GetContextMenu(Menu, Index);
}


// Layouts

#define CenterRow(Last) for (INT Index=RowStart; Index<=Last; Index++) \
	{ \
		TagcloudItemData* pData = GetTagcloudItemData(Index); \
		if (pData->Hdr.Valid) \
		{ \
			OffsetRect(&pData->Hdr.Rect, (rectLayout.Width()+GUTTER-x)/2, (RowHeight-(pData->Hdr.Rect.bottom-pData->Hdr.Rect.top))/2); \
			if (pData->Hdr.Rect.right>m_ScrollWidth) \
				m_ScrollWidth = pData->Hdr.Rect.right; \
			if (pData->Hdr.Rect.bottom+MARGIN>m_ScrollHeight) \
				if (((m_ScrollHeight=pData->Hdr.Rect.bottom+MARGIN)>rectLayout.Height()) && !HasScrollbars) \
				{ \
					rectLayout.right -= GetSystemMetrics(SM_CXVSCROLL); \
					HasScrollbars = TRUE; \
					goto Restart; \
				} \
		} \
	}

void CTagcloudView::AdjustLayout()
{
	// Layout rect
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	// DC
	CClientDC dc(this);

	// Items
	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = 0;
	INT y = MARGIN;
	INT RowHeight = 0;
	INT RowStart = 0;

	for (INT a=0; a<m_ItemCount; a++)
	{
		TagcloudItemData* pData = GetTagcloudItemData(a);

		if (pData->Hdr.Valid)
		{
			const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];

			CRect rect(0, 0, rectLayout.Width()-2*MARGIN, 128);
			dc.SelectObject(GetItemFont(a));
			dc.DrawText(pItemDescriptor->CoreAttributes.FileName, LFHasSubcaption(pItemDescriptor) ? pItemDescriptor->FolderMainCaptionCount : -1, rect, TEXTFORMAT | DT_CALCRECT);
			rect.InflateRect(PADDINGX, PADDINGY);

			// Next row
			if (x+rect.Width()+2*MARGIN>rectLayout.Width())
			{
				Column = 0;
				Row++;
				y += RowHeight+GUTTER;

				if (a)
					CenterRow(a-1);

				x = 0;
				RowStart = a;
				RowHeight = 0;
			}

			rect.MoveToXY(x, y);

			pData->Hdr.Rect = rect;
			pData->Hdr.Column = Column++;
			pData->Hdr.Row = Row;

			x += rect.Width()+GUTTER;
			RowHeight = max(RowHeight, rect.Height());
		}
	}

	if (m_ItemCount)
		CenterRow(m_ItemCount-1);

	CFileView::AdjustLayout();
}


// Item handling

COLORREF CTagcloudView::GetItemTextColor(INT Index, BOOL Themed) const
{
	const TagcloudItemData* pData = GetTagcloudItemData(Index);

	COLORREF clrText = m_GlobalViewSettings.TagcloudUseColor ? pData->Color : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);

	if (m_GlobalViewSettings.TagcloudUseOpacity)
	{
		const COLORREF clrBack = GetStageBackgroundColor(Themed);

		clrText = ((clrText & 0xFF)*pData->Alpha + (clrBack & 0xFF)*(255-pData->Alpha))>>8 |
			((((clrText>>8) & 0xFF)*pData->Alpha + ((clrBack>>8) & 0xFF)*(255-pData->Alpha)) & 0xFF00) |
			((((clrText>>16) & 0xFF)*pData->Alpha + ((clrBack>>16) & 0xFF)*(255-pData->Alpha))<<8) & 0xFF0000;
	}

	return clrText;
}


// Drawing

void CTagcloudView::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL /*Themed*/)
{
	ASSERT(rectItem);

	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	CFont* pOldFont = dc.SelectObject(GetItemFont(Index));
	dc.DrawText(pItemDescriptor->CoreAttributes.FileName, LFHasSubcaption(pItemDescriptor) ? pItemDescriptor->FolderMainCaptionCount : -1, (LPRECT)rectItem, TEXTFORMAT | DT_CENTER | DT_VCENTER);
	dc.SelectObject(pOldFont);
}


// Font sizes

LFFont* CTagcloudView::GetItemFont(INT Index)
{
	return &m_Fonts[m_GlobalViewSettings.TagcloudUseSize ? GetTagcloudItemData(Index)->FontSize : DEFAULTFONT];
}


BEGIN_MESSAGE_MAP(CTagcloudView, CFileView)
	ON_WM_CREATE()
	ON_COMMAND(IDM_TAGCLOUD_SORTVALUE, OnSortValue)
	ON_COMMAND(IDM_TAGCLOUD_SORTCOUNT, OnSortCount)
	ON_COMMAND(IDM_TAGCLOUD_SHOWRARE, OnShowRare)
	ON_COMMAND(IDM_TAGCLOUD_USESIZE, OnUseSize)
	ON_COMMAND(IDM_TAGCLOUD_USECOLOR, OnUseColor)
	ON_COMMAND(IDM_TAGCLOUD_USEOPACITY, OnUseOpacity)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TAGCLOUD_SORTVALUE, IDM_TAGCLOUD_USEOPACITY, OnUpdateCommands)
END_MESSAGE_MAP()

INT CTagcloudView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Font sizes
	for (INT a=0; a<20; a++)
		m_Fonts[a].CreateFont(a*2+10, a>=6 ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY);

	return 0;
}


void CTagcloudView::OnSortValue()
{
	p_GlobalViewSettings->TagcloudSort = 0;

	theApp.UpdateViewSettings(-1, LFViewTagcloud);
}

void CTagcloudView::OnSortCount()
{
	p_GlobalViewSettings->TagcloudSort = 1;

	theApp.UpdateViewSettings(-1, LFViewTagcloud);
}

void CTagcloudView::OnShowRare()
{
	p_GlobalViewSettings->TagcloudShowRare = !p_GlobalViewSettings->TagcloudShowRare;

	theApp.UpdateViewSettings(-1, LFViewTagcloud);
}

void CTagcloudView::OnUseSize()
{
	p_GlobalViewSettings->TagcloudUseSize = !p_GlobalViewSettings->TagcloudUseSize;

	theApp.UpdateViewSettings(-1, LFViewTagcloud);
}

void CTagcloudView::OnUseColor()
{
	p_GlobalViewSettings->TagcloudUseColor = !p_GlobalViewSettings->TagcloudUseColor;

	theApp.UpdateViewSettings(-1, LFViewTagcloud);
}

void CTagcloudView::OnUseOpacity()
{
	p_GlobalViewSettings->TagcloudUseOpacity = !p_GlobalViewSettings->TagcloudUseOpacity;

	theApp.UpdateViewSettings(-1, LFViewTagcloud);
}

void CTagcloudView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_TAGCLOUD_SORTVALUE:
		pCmdUI->SetCheck(m_GlobalViewSettings.TagcloudSort==0);

		if (!pCmdUI->m_pMenu)
			bEnable = (m_GlobalViewSettings.TagcloudSort!=0) && !m_Nothing;

		break;

	case IDM_TAGCLOUD_SORTCOUNT:
		pCmdUI->SetCheck(m_GlobalViewSettings.TagcloudSort==1);

		if (!pCmdUI->m_pMenu)
			bEnable = (m_GlobalViewSettings.TagcloudSort!=1) && !m_Nothing;

		break;

	case IDM_TAGCLOUD_SHOWRARE:
		pCmdUI->SetCheck(m_GlobalViewSettings.TagcloudShowRare);
		break;

	case IDM_TAGCLOUD_USESIZE:
		pCmdUI->SetCheck(m_GlobalViewSettings.TagcloudUseSize);
		break;

	case IDM_TAGCLOUD_USECOLOR:
		pCmdUI->SetCheck(m_GlobalViewSettings.TagcloudUseColor);
		break;

	case IDM_TAGCLOUD_USEOPACITY:
		pCmdUI->SetCheck(m_GlobalViewSettings.TagcloudUseOpacity);
		break;
	}

	pCmdUI->Enable(bEnable);
}
