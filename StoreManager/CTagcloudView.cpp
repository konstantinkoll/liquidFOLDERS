
// CTagcloudView.cpp: Implementierung der Klasse CTagcloudView
//

#include "stdafx.h"
#include "CTagcloudView.h"
#include "StoreManager.h"


// CTagcloudView
//

#define GetItemData(idx)     ((TagcloudItemData*)(m_ItemData+(idx)*m_DataSize))
#define DefaultFontSize      2
#define TextFormat           DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE
#define GUTTER               3


CTagcloudView::CTagcloudView()
	: CGridView(sizeof(TagcloudItemData))
{
}

void CTagcloudView::SetViewOptions(BOOL Force)
{
	UINT Changes = 0;

	if ((Force) || (m_ViewParameters.TagcloudCanonical!=p_ViewParameters->TagcloudCanonical))
		Changes = 2;
	if ((Force) || (m_ViewParameters.TagcloudHideRare!=p_ViewParameters->TagcloudHideRare))
		Changes = 2;
	if ((Force) || (m_ViewParameters.TagcloudUseSize!=p_ViewParameters->TagcloudUseSize))
		Changes = 1;

	if (p_Result)
	{
		m_ViewParameters = *p_ViewParameters;

		switch (Changes)
		{
		case 2:
			SetSearchResult(p_Result);
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

void CTagcloudView::SetSearchResult(LFSearchResult* Result)
{
	if (Result)
	{
		LFSortSearchResult(Result, m_ViewParameters.TagcloudCanonical ? m_ViewParameters.SortBy : LFAttrFileCount, m_ViewParameters.TagcloudCanonical==FALSE);

		INT Minimum = -1;
		INT Maximum = -1;

		// Find items
		for (UINT a=0; a<Result->m_ItemCount; a++)
		{
			LFItemDescriptor* i = Result->m_Items[a];
			TagcloudItemData* d = GetItemData(a);

			if ((i->Type & LFTypeMask)==LFTypeVirtual)
				d->Cnt = i->AggregateCount;

			if (d->Cnt)
			{
				Minimum = (Minimum==-1) ? d->Cnt : min(Minimum, d->Cnt);
				Maximum = (Maximum==-1) ? d->Cnt : max(Maximum, d->Cnt);
			}
		}

		// Calculate display properties
		INT Delta = Maximum-Minimum+1;
		for (UINT a=0; a<Result->m_ItemCount; a++)
		{
			TagcloudItemData* d = GetItemData(a);
			if (d->Cnt)
				if ((m_ViewParameters.TagcloudHideRare) && (Delta>1) && (d->Cnt==Minimum))
				{
					ZeroMemory(d, sizeof(TagcloudItemData));
					d->Hdr.Hdr.SysIconIndex = -1;
				}
				else
					if (Delta==1)
					{
						d->FontSize = 19;
						d->Alpha = 255;
						d->Color = 0xFF0000;
					}
					else
					{
						d->FontSize = (20*(d->Cnt-Minimum))/Delta;
						d->Alpha = 56+(200*(d->Cnt-Minimum))/Delta;
						if (d->Alpha<128)
						{
							d->Color = 0xFF0000 | ((128-d->Alpha)<<9);
						}
						else
							if (d->Alpha<192)
							{
								d->Color = (0xFF0000-(d->Alpha-128)*0x020000) | ((d->Alpha-128)*2);
							}
							else
							{
								d->Color = (0x800000-(d->Alpha-192)*0x020000) | (0x000080+(d->Alpha-192)*2);
							}
					}
		}

		// Focus item
		if (!GetItemData(m_FocusItem)->Cnt)
			for (UINT a=0; a<Result->m_ItemCount; a++)
				if (GetItemData(a)->Cnt)
				{
					m_FocusItem = a;
					break;
				}
	}
}

void CTagcloudView::AdjustLayout()
{
	ResetItemCategories();

	if (p_Result)
	{
		CClientDC dc(this);

		CRect rectWindow;
		GetWindowRect(&rectWindow);
		if (!rectWindow.Width())
			return;

#define CenterRow(last) for (UINT b=rowstart; b<=last; b++) \
	{ \
		TagcloudItemData* d = GetItemData(b); \
		if (d->Cnt) \
		{ \
			OffsetRect(&d->Hdr.Hdr.Rect, (rectWindow.Width()+GUTTER-x)/2, (rowheight-(d->Hdr.Hdr.Rect.bottom-d->Hdr.Hdr.Rect.top))/2); \
			if (d->Hdr.Hdr.Rect.right-1>m_ScrollWidth) \
				m_ScrollWidth = d->Hdr.Hdr.Rect.right-1; \
			if (d->Hdr.Hdr.Rect.bottom-1>m_ScrollHeight) \
			{ \
				m_ScrollHeight = d->Hdr.Hdr.Rect.bottom-1; \
				if ((m_ScrollHeight>=rectWindow.Height()) && (!HasScrollbars)) \
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

		INT col = 0;
		INT row = 0;
		INT x = 0;
		INT y = GUTTER;
		INT rowheight = 0;
		INT rowstart = 0;

		for (UINT a=0; a<p_Result->m_ItemCount; a++)
		{
			TagcloudItemData* d = GetItemData(a);
			if (d->Cnt)
			{
				LFItemDescriptor* i = p_Result->m_Items[a];

				CRect rect(0, 0, rectWindow.Width()-2*GUTTER, 128);
				dc.SelectObject(GetFont(a));
				dc.DrawText(i->CoreAttributes.FileName, -1, rect, TextFormat | DT_CALCRECT);
				rect.InflateRect(5, 4);

				if (x+rect.Width()+2*GUTTER>rectWindow.Width())
				{
					col = 0;
					row++;
					y += rowheight+GUTTER;
					if (a)
						CenterRow(a-1);
					x = 0;
					rowstart = a;
					rowheight = 0;
				}

				rect.MoveToXY(x, y);
				d->Hdr.Column = col++;
				d->Hdr.Row = row;
				d->Hdr.Hdr.Rect = rect;

				x += rect.Width()+GUTTER;
				rowheight = max(rowheight, rect.Height());
			}
		}

		if (p_Result->m_ItemCount)
			CenterRow(p_Result->m_ItemCount-1);
	}
	else
	{
		m_ScrollWidth = m_ScrollHeight = 0;
	}

	m_GridArrange = GRIDARRANGE_HORIZONTAL;
	AdjustScrollbars();
	Invalidate();
}

void CTagcloudView::DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed)
{
	LFItemDescriptor* i = p_Result->m_Items[idx];
	TagcloudItemData* d = GetItemData(idx);

	COLORREF color = m_ViewParameters.TagcloudUseColors ? d->Color : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);

	if ((d->Hdr.Hdr.Selected) && (!hThemeList))
	{
		color = GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT);
	}
	else
		if (m_ViewParameters.TagcloudUseOpacity)
		{
			COLORREF back = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
			color = ((color & 0xFF)*d->Alpha + (back & 0xFF)*(255-d->Alpha))>>8 |
				((((color>>8) & 0xFF)*d->Alpha + ((back>>8) & 0xFF)*(255-d->Alpha)) & 0xFF00) |
				((((color>>16) & 0xFF)*d->Alpha + ((back>>16) & 0xFF)*(255-d->Alpha))<<8) & 0xFF0000;
		}

	CFont* pOldFont = dc.SelectObject(GetFont(idx));
	dc.SetTextColor(color);
	dc.DrawText(i->CoreAttributes.FileName, -1, rectItem, TextFormat | DT_CENTER | DT_VCENTER);
	dc.SelectObject(pOldFont);
}

CMenu* CTagcloudView::GetBackgroundContextMenu()
{
	CMenu* menu = new CMenu();
	menu->LoadMenu(IDM_TAGCLOUD);
	return menu;
}

CFont* CTagcloudView::GetFont(INT idx)
{
	return &m_Fonts[(m_ViewParameters.TagcloudUseSize ? GetItemData(idx)->FontSize : DefaultFontSize)];
}


BEGIN_MESSAGE_MAP(CTagcloudView, CGridView)
	ON_WM_CREATE()
	ON_COMMAND(IDM_TAGCLOUD_SORTVALUE, OnSortValue)
	ON_COMMAND(IDM_TAGCLOUD_SORTCOUNT, OnSortCount)
	ON_COMMAND(IDM_TAGCLOUD_HIDERARE, OnHideRare)
	ON_COMMAND(IDM_TAGCLOUD_USESIZE, OnUseSize)
	ON_COMMAND(IDM_TAGCLOUD_USECOLORS, OnUseColors)
	ON_COMMAND(IDM_TAGCLOUD_USEOPACITY, OnUseOpacity)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TAGCLOUD_SORTVALUE, IDM_TAGCLOUD_USEOPACITY, OnUpdateCommands)
END_MESSAGE_MAP()

INT CTagcloudView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGridView::OnCreate(lpCreateStruct)==-1)
		return -1;

	CString Face = theApp.GetDefaultFontFace();
	for (INT a=0; a<20; a++)
		m_Fonts[a].CreateFont(-(a*2+10), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, a>=4 ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, Face);

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

void CTagcloudView::OnHideRare()
{
	p_ViewParameters->TagcloudHideRare = !p_ViewParameters->TagcloudHideRare;
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
		pCmdUI->SetCheck(m_ViewParameters.TagcloudCanonical);
		if (!pCmdUI->m_pMenu)
			b = !m_ViewParameters.TagcloudCanonical;
		break;
	case IDM_TAGCLOUD_SORTCOUNT:
		pCmdUI->SetCheck(!m_ViewParameters.TagcloudCanonical);
		if (!pCmdUI->m_pMenu)
			b = m_ViewParameters.TagcloudCanonical;
		break;
	case IDM_TAGCLOUD_HIDERARE:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudHideRare);
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
