
// CTagcloudView.cpp: Implementierung der Klasse CTagcloudView
//

#include "stdafx.h"
#include "CTagcloudView.h"
#include "liquidFOLDERS.h"


// CTagcloudView
//

#define GetItemData(Index)     ((TagcloudItemData*)(m_ItemData+(Index)*m_DataSize))
#define DefaultFontSize        2
#define TextFormat             DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE
#define GUTTER                 3
#define MARGIN                 BACKSTAGEBORDER


CTagcloudView::CTagcloudView()
	: CGridView(sizeof(TagcloudItemData), FALSE)
{
}

void CTagcloudView::SetViewOptions(BOOL Force)
{
	UINT Changes = 0;

	if ((Force) || (m_ViewParameters.TagcloudUseSize!=p_ViewParameters->TagcloudUseSize) || (m_ViewParameters.TagcloudUseColors!=p_ViewParameters->TagcloudUseColors))
		Changes = 1;

	if ((Force) || (m_ViewParameters.TagcloudCanonical!=p_ViewParameters->TagcloudCanonical))
		Changes = 2;

	if ((Force) || (m_ViewParameters.TagcloudShowRare!=p_ViewParameters->TagcloudShowRare))
		Changes = 2;

	if (p_CookedFiles)
	{
		m_ViewParameters = *p_ViewParameters;

		switch (Changes)
		{
		case 2:
			SetSearchResult(p_RawFiles, p_CookedFiles, NULL);
			GetOwner()->PostMessage(WM_COMMAND, WM_UPDATESELECTION);

		case 1:
			AdjustLayout();
			break;

		case 0:
			Invalidate();
			break;
		}
	}
}

void CTagcloudView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	CGridView::SetSearchResult(pRawFiles, pCookedFiles, Data);

	if (p_CookedFiles)
	{
		LFSortSearchResult(p_CookedFiles, m_ViewParameters.TagcloudCanonical ? m_ViewParameters.SortBy : LFAttrFileCount,
			(m_ViewParameters.TagcloudCanonical==FALSE) || (theApp.m_Attributes[m_ViewParameters.SortBy].PreferDescendingSort));

		INT Minimum = -1;
		INT Maximum = -1;

		// Find items
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			LFItemDescriptor* i = p_CookedFiles->m_Items[a];
			TagcloudItemData* pData = GetItemData(a);

			if ((i->Type & LFTypeMask)==LFTypeFolder)
				pData->Cnt = i->AggregateCount;

			pData->Hdr.Hdr.Valid = pData->Cnt;
			if (pData->Hdr.Hdr.Valid)
			{
				Minimum = (Minimum==-1) ? pData->Cnt : min(Minimum, pData->Cnt);
				Maximum = (Maximum==-1) ? pData->Cnt : max(Maximum, pData->Cnt);
			}
		}

		// Calculate display properties
		INT Delta = Maximum-Minimum+1;
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TagcloudItemData* pData = GetItemData(a);
			if (pData->Hdr.Hdr.Valid)
				if ((!m_ViewParameters.TagcloudShowRare) && (Delta>1) && (pData->Cnt==Minimum))
				{
					ZeroMemory(pData, sizeof(TagcloudItemData));
				}
				else
					if (Delta==1)
					{
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

void CTagcloudView::AdjustLayout()
{
	ResetItemCategories();

	if (p_CookedFiles)
	{
		CClientDC dc(this);

		CRect rectWindow;
		GetWindowRect(rectWindow);

		if (!rectWindow.Width())
			return;

#define CenterRow(last) for (UINT b=RowStart; b<=last; b++) \
	{ \
		TagcloudItemData* pData = GetItemData(b); \
		if (pData->Hdr.Hdr.Valid) \
		{ \
			OffsetRect(&pData->Hdr.Hdr.Rect, (rectWindow.Width()+GUTTER-x)/2, (RowHeight-(pData->Hdr.Hdr.Rect.bottom-pData->Hdr.Hdr.Rect.top))/2); \
			if (pData->Hdr.Hdr.Rect.right>m_ScrollWidth) \
				m_ScrollWidth = pData->Hdr.Hdr.Rect.right; \
			if (pData->Hdr.Hdr.Rect.bottom+MARGIN>m_ScrollHeight) \
			{ \
				m_ScrollHeight = pData->Hdr.Hdr.Rect.bottom+MARGIN; \
				if ((m_ScrollHeight>rectWindow.Height()) && (!HasScrollbars)) \
				{ \
					HasScrollbars = TRUE; \
					rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL); \
					goto Restart; \
				} \
			} \
		} \
	}

		BOOL HasScrollbars = FALSE;

Restart:
		m_ScrollWidth = m_ScrollHeight = 0;

		INT Column = 0;
		INT Row = 0;
		INT x = 0;
		INT y = MARGIN;
		INT RowHeight = 0;
		INT RowStart = 0;

		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TagcloudItemData* pData = GetItemData(a);
			if (pData->Hdr.Hdr.Valid)
			{
				LFItemDescriptor* i = p_CookedFiles->m_Items[a];

				CRect rect(0, 0, rectWindow.Width()-2*MARGIN, 128);
				dc.SelectObject(GetFont(a));
				dc.DrawText(i->CoreAttributes.FileName, rect, TextFormat | DT_CALCRECT);
				rect.InflateRect(5, 4);

				if (x+rect.Width()+2*MARGIN>rectWindow.Width())
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
				pData->Hdr.Column = Column++;
				pData->Hdr.Row = Row;
				pData->Hdr.Hdr.Rect = rect;

				x += rect.Width()+GUTTER;
				RowHeight = max(RowHeight, rect.Height());
			}
		}

		if (p_CookedFiles->m_ItemCount)
			CenterRow(p_CookedFiles->m_ItemCount-1);
	}
	else
	{
		m_ScrollWidth = m_ScrollHeight = 0;
	}

	m_GridArrange = GRIDARRANGE_HORIZONTAL;

	CFileView::AdjustLayout();
}

void CTagcloudView::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* i = p_CookedFiles->m_Items[Index];
	TagcloudItemData* pData = GetItemData(Index);

	if (!pData->Hdr.Hdr.Selected)
	{
		COLORREF clr = m_ViewParameters.TagcloudUseColors ? pData->Color : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);

		if (m_ViewParameters.TagcloudUseOpacity)
		{
			const COLORREF clrBack = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
			clr = ((clr & 0xFF)*pData->Alpha + (clrBack & 0xFF)*(255-pData->Alpha))>>8 |
				((((clr>>8) & 0xFF)*pData->Alpha + ((clrBack>>8) & 0xFF)*(255-pData->Alpha)) & 0xFF00) |
				((((clr>>16) & 0xFF)*pData->Alpha + ((clrBack>>16) & 0xFF)*(255-pData->Alpha))<<8) & 0xFF0000;
		}

		dc.SetTextColor(clr);
	}

	CFont* pOldFont = dc.SelectObject(GetFont(Index));
	dc.DrawText(i->CoreAttributes.FileName, (LPRECT)rectItem, TextFormat | DT_CENTER | DT_VCENTER);
	dc.SelectObject(pOldFont);
}

CMenu* CTagcloudView::GetViewContextMenu()
{
	CMenu* pMenu = new CMenu();
	pMenu->LoadMenu(IDM_TAGCLOUD);

	return pMenu;
}

LFFont* CTagcloudView::GetFont(INT Index)
{
	return &m_Fonts[(m_ViewParameters.TagcloudUseSize ? GetItemData(Index)->FontSize : DefaultFontSize)];
}


BEGIN_MESSAGE_MAP(CTagcloudView, CGridView)
	ON_WM_CREATE()
	ON_COMMAND(IDM_TAGCLOUD_SORTVALUE, OnSortValue)
	ON_COMMAND(IDM_TAGCLOUD_SORTCOUNT, OnSortCount)
	ON_COMMAND(IDM_TAGCLOUD_SHOWRARE, OnShowRare)
	ON_COMMAND(IDM_TAGCLOUD_USESIZE, OnUseSize)
	ON_COMMAND(IDM_TAGCLOUD_USECOLORS, OnUseColors)
	ON_COMMAND(IDM_TAGCLOUD_USEOPACITY, OnUseOpacity)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TAGCLOUD_SORTVALUE, IDM_TAGCLOUD_USEOPACITY, OnUpdateCommands)
END_MESSAGE_MAP()

INT CTagcloudView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGridView::OnCreate(lpCreateStruct)==-1)
		return -1;

	for (INT a=0; a<20; a++)
		m_Fonts[a].CreateFont(a*2+10, a>=6 ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY);

	return 0;
}

void CTagcloudView::OnSortValue()
{
	p_ViewParameters->TagcloudCanonical = TRUE;
	theApp.UpdateViewOptions(m_Context);
}

void CTagcloudView::OnSortCount()
{
	p_ViewParameters->TagcloudCanonical = FALSE;
	theApp.UpdateViewOptions(m_Context);
}

void CTagcloudView::OnShowRare()
{
	p_ViewParameters->TagcloudShowRare = !p_ViewParameters->TagcloudShowRare;
	theApp.UpdateViewOptions(m_Context);
}

void CTagcloudView::OnUseSize()
{
	p_ViewParameters->TagcloudUseSize = !p_ViewParameters->TagcloudUseSize;
	theApp.UpdateViewOptions(m_Context);
}

void CTagcloudView::OnUseColors()
{
	p_ViewParameters->TagcloudUseColors = !p_ViewParameters->TagcloudUseColors;
	theApp.UpdateViewOptions(m_Context);
}

void CTagcloudView::OnUseOpacity()
{
	p_ViewParameters->TagcloudUseOpacity = !p_ViewParameters->TagcloudUseOpacity;
	theApp.UpdateViewOptions(m_Context);
}

void CTagcloudView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case IDM_TAGCLOUD_SORTVALUE:
		pCmdUI->SetRadio(m_ViewParameters.TagcloudCanonical);
		if (!pCmdUI->m_pMenu)
			b = !m_ViewParameters.TagcloudCanonical && !m_Nothing;
		break;
	case IDM_TAGCLOUD_SORTCOUNT:
		pCmdUI->SetRadio(!m_ViewParameters.TagcloudCanonical);
		if (!pCmdUI->m_pMenu)
			b = m_ViewParameters.TagcloudCanonical && !m_Nothing;
		break;
	case IDM_TAGCLOUD_SHOWRARE:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudShowRare);
		break;
	case IDM_TAGCLOUD_USESIZE:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudUseSize);
		break;
	case IDM_TAGCLOUD_USECOLORS:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudUseColors);
		break;
	case IDM_TAGCLOUD_USEOPACITY:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudUseOpacity);
	}

	pCmdUI->Enable(b);
}
