
// CTagcloudView.cpp: Implementierung der Klasse CTagcloudView
//

#include "stdafx.h"
#include "CTagcloudView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CTagcloudView
//

#define DefaultFontSize     2
#define TextFormat          DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE
#define Gutter              2


CTagcloudView::CTagcloudView()
{
	m_Tags = NULL;
	hTheme = NULL;

	CString face = theApp.GetDefaultFontFace();
	for (int a=0; a<22; a++)
		m_Fonts[a].CreateFont(-(a*2+10), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, a>=4 ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, face);

}

CTagcloudView::~CTagcloudView()
{
	if (m_Tags)
		delete[] m_Tags;
}

void CTagcloudView::Create(CWnd* _pParentWnd, LFSearchResult* _result, int _FocusItem)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewTagcloud, _FocusItem, theApp.OSVersion>=OS_Vista);
}

void CTagcloudView::SetViewOptions(UINT /*_ViewID*/, BOOL Force)
{
	UINT Changes = 0;

	if ((Force) || (m_ViewParameters.TagcloudCanonical!=pViewParameters->TagcloudCanonical))
		Changes = 2;
	if ((Force) || (m_ViewParameters.TagcloudOmitRare!=pViewParameters->TagcloudOmitRare))
		Changes = 2;
	if ((Force) || (m_ViewParameters.TagcloudUseSize!=pViewParameters->TagcloudUseSize))
		Changes = 1;

	m_ViewParameters = *pViewParameters;

	switch (Changes)
	{
	case 1:
		AdjustLayout();
	case 0:
		Invalidate();
		break;
	case 2:
		SetSearchResult(result);
		break;
	}
}

void CTagcloudView::SetSearchResult(LFSearchResult* _result)
{
	UINT VictimCount = result ? result->m_ItemCount : 0;
	Tag* Victim = m_Tags;
	m_Tags = NULL;

	if (_result)
		if (_result->m_ItemCount)
		{
			LFSortSearchResult(_result, m_ViewParameters.TagcloudCanonical ? m_ViewParameters.SortBy : LFAttrFileCount, m_ViewParameters.TagcloudCanonical==FALSE);
			m_Tags = new Tag[_result->m_ItemCount];

			int minimum = -1;
			int maximum = -1;

			// Find items
			for (UINT a=0; a<_result->m_ItemCount; a++)
			{
				ZeroMemory(&m_Tags[a], sizeof(Tag));
				if (a<VictimCount)
					m_Tags[a].selected = Victim[a].selected;

				LFItemDescriptor* i = _result->m_Items[a];
				if ((i->Type & LFTypeMask)==LFTypeVirtual)
					m_Tags[a].cnt = i->AggregateCount;

				if (m_Tags[a].cnt)
				{
					minimum = (minimum==-1) ? m_Tags[a].cnt : min(minimum, m_Tags[a].cnt);
					maximum = (maximum==-1) ? m_Tags[a].cnt : max(maximum, m_Tags[a].cnt);
				}
			}

			// Calculate display properties
			int delta = maximum-minimum+1;
			for (UINT a=0; a<_result->m_ItemCount; a++)
				if (m_Tags[a].cnt)
					if ((m_ViewParameters.TagcloudOmitRare) && (delta>1) && (m_Tags[a].cnt==minimum))
					{
						m_Tags[a].cnt = 0;
					}
					else
						if (delta==1)
						{
							m_Tags[a].fontsize = 19;
							m_Tags[a].alpha = 255;
							m_Tags[a].color = 0x0000FF;
						}
						else
						{
							m_Tags[a].fontsize = (20*(m_Tags[a].cnt-minimum))/delta;
							m_Tags[a].alpha = 56+(200*(m_Tags[a].cnt-minimum))/delta;
							if (m_Tags[a].alpha<128)
							{
								m_Tags[a].color = 0xFF0000 | ((128-m_Tags[a].alpha)<<9);
							}
							else
								if (m_Tags[a].alpha<192)
								{
									m_Tags[a].color = (0xFF0000-(m_Tags[a].alpha-128)*0x020000) | ((m_Tags[a].alpha-128)*2);
								}
								else
								{
									m_Tags[a].color = (0x800000-(m_Tags[a].alpha-192)*0x020000) | (0x000080+(m_Tags[a].alpha-192)*2);
								}
						}

			// Set focus
			if (!m_Tags[FocusItem].cnt)
				for (UINT a=(UINT)FocusItem; a<_result->m_ItemCount; a++)
					if (m_Tags[a].cnt)
					{
						FocusItem = a;
						break;
					}
			if (!m_Tags[FocusItem].cnt)
				for (UINT a=0; a<_result->m_ItemCount; a++)
					if (m_Tags[a].cnt)
					{
						FocusItem = a;
						break;
					}
		}

	result = _result;
	if (Victim)
		delete[] Victim;

	AdjustLayout();
	Invalidate();
}

void CTagcloudView::SelectItem(int n, BOOL select, BOOL InternalCall)
{
	if (m_Tags)
	{
		if (m_Tags[n].cnt)
			m_Tags[n].selected = select;

		if (!InternalCall)
		{
			Invalidate();
			GetParentFrame()->SendMessage(WM_COMMAND, ID_APP_UPDATESELECTION);
		}
	}
}

int CTagcloudView::GetSelectedItem()
{
	if ((m_Tags) && (FocusItem!=-1))
		if (m_Tags[FocusItem].selected)
			return FocusItem;

	return -1;
}

int CTagcloudView::GetNextSelectedItem(int n)
{
	if (m_Tags)
	{
		if (n<-1)
			n = -1;

		for (UINT a=(UINT)(n+1); a<result->m_ItemCount; a++)
			if (m_Tags[a].selected)
				return a;
	}

	return -1;
}

BOOL CTagcloudView::IsSelected(int n)
{
	return (m_Tags && (n>=0)? m_Tags[n].selected : FALSE);
}

int CTagcloudView::ItemAtPosition(CPoint point)
{
	if ((m_Tags) && (result))
		for (UINT a=0; a<result->m_ItemCount; a++)
			if (m_Tags[a].cnt)
			{
				CRect rect(m_Tags[a].rect);
				if (rect.PtInRect(point))
					return a;
			}

	return -1;
}

void CTagcloudView::InvalidateItem(int n)
{
	if ((m_Tags) && (result) && (n!=-1))
		InvalidateRect(&m_Tags[n].rect, FALSE);
}

CMenu* CTagcloudView::GetContextMenu()
{
	CMenu* menu = new CMenu();
	menu->LoadMenu(IDM_TAGCLOUD);
	return menu;
}

CFont* CTagcloudView::GetFont(int idx)
{
	return &m_Fonts[(m_ViewParameters.TagcloudUseSize ? m_Tags[idx].fontsize : DefaultFontSize) + (m_ViewParameters.GrannyMode ? 2 : 0)];
}

void CTagcloudView::AdjustLayout()
{
	if (result)
	{
		CClientDC dc(this);

		CRect rectClient;
		GetClientRect(rectClient);
		if (!rectClient.Width())
			return;

		int row = Gutter;
		int col = 0;
		int rowheight = 0;
		int rowstart = 0;

#define CenterRow(last) for (UINT b=rowstart; b<=last; b++) \
	if (m_Tags[b].cnt) { \
	int offs = (rowheight-(m_Tags[b].rect.bottom-m_Tags[b].rect.top))/2; m_Tags[b].rect.bottom += offs; m_Tags[b].rect.top += offs; \
	offs = (rectClient.Width()-col)/2; m_Tags[b].rect.left += offs; m_Tags[b].rect.right += offs; \
	}

		for (UINT a=0; a<result->m_ItemCount; a++)
			if (m_Tags[a].cnt)
			{
				CRect rect(0, 0, rectClient.Width()-2*Gutter, 128);
				dc.SelectObject(GetFont(a));
				dc.DrawText(result->m_Items[a]->CoreAttributes.FileName, -1, rect, TextFormat | DT_CALCRECT);
				rect.InflateRect(5, 4);

				if (col+rect.Width()+3*Gutter>rectClient.Width())
				{
					row += rowheight+Gutter;
					if (a)
						CenterRow(a-1);
					col = 0;
					rowstart = a;
					rowheight = 0;
				}

				rect.MoveToXY(col, row);
				m_Tags[a].rect = rect;

				col += rect.Width()+Gutter;
				rowheight = max(rowheight, rect.Height());
			}

		if (result->m_ItemCount)
			CenterRow(result->m_ItemCount-1);
	}
}


BEGIN_MESSAGE_MAP(CTagcloudView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_TAGCLOUD_SORTVALUE, OnSortValue)
	ON_COMMAND(ID_TAGCLOUD_SORTCOUNT, OnSortCount)
	ON_COMMAND(ID_TAGCLOUD_OMITRARE, OnOmitRare)
	ON_COMMAND(ID_TAGCLOUD_USESIZE, OnUseSize)
	ON_COMMAND(ID_TAGCLOUD_USECOLORS, OnUseColors)
	ON_COMMAND(ID_TAGCLOUD_USEOPACITY, OnUseOpacity)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TAGCLOUD_SORTVALUE, ID_TAGCLOUD_USEOPACITY, OnUpdateCommands)
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

int CTagcloudView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((theApp.m_ThemeLibLoaded) && (theApp.OSVersion>=OS_Vista))
	{
		theApp.zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
		hTheme = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	return 0;
}

void CTagcloudView::OnDestroy()
{
	if (hTheme)
		theApp.zCloseThemeData(hTheme);

	CFileView::OnDestroy();
}

void CTagcloudView::OnSortValue()
{
	pViewParameters->TagcloudCanonical = TRUE;
	OnViewOptionsChanged();
}

void CTagcloudView::OnSortCount()
{
	pViewParameters->TagcloudCanonical = FALSE;
	OnViewOptionsChanged();
}

void CTagcloudView::OnOmitRare()
{
	pViewParameters->TagcloudOmitRare = !pViewParameters->TagcloudOmitRare;
	OnViewOptionsChanged();
}

void CTagcloudView::OnUseSize()
{
	pViewParameters->TagcloudUseSize = !pViewParameters->TagcloudUseSize;
	OnViewOptionsChanged();
}

void CTagcloudView::OnUseColors()
{
	pViewParameters->TagcloudUseColors = !pViewParameters->TagcloudUseColors;
	OnViewOptionsChanged();
}

void CTagcloudView::OnUseOpacity()
{
	pViewParameters->TagcloudUseOpacity = !pViewParameters->TagcloudUseOpacity;
	OnViewOptionsChanged();
}

void CTagcloudView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;
	switch (pCmdUI->m_nID)
	{
	case ID_TAGCLOUD_SORTVALUE:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudCanonical);
		break;
	case ID_TAGCLOUD_SORTCOUNT:
		pCmdUI->SetCheck(!m_ViewParameters.TagcloudCanonical);
		break;
	case ID_TAGCLOUD_OMITRARE:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudOmitRare);
		break;
	case ID_TAGCLOUD_USESIZE:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudUseSize);
		break;
	case ID_TAGCLOUD_USECOLORS:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudUseColors);
		break;
	case ID_TAGCLOUD_USEOPACITY:
		pCmdUI->SetCheck(m_ViewParameters.TagcloudUseOpacity);
	}

	pCmdUI->Enable(b);
}

LRESULT CTagcloudView::OnThemeChanged()
{
	if ((theApp.m_ThemeLibLoaded) && (theApp.OSVersion>=OS_Vista))
	{
		if (hTheme)
			theApp.zCloseThemeData(hTheme);

		hTheme = theApp.zOpenThemeData(m_hWnd, VSCLASS_LISTVIEW);
	}

	return TRUE;
}

void CTagcloudView::OnSize(UINT nType, int cx, int cy)
{
	CFileView::OnSize(nType, cx, cy);

	AdjustLayout();
	Invalidate();
}

BOOL CTagcloudView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTagcloudView::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	COLORREF back;
	COLORREF text;
	theApp.GetBackgroundColors(m_ViewParameters.Background, &back, &text);

	dc.FillSolidRect(rect, back);

	if (result)
		for (UINT a=0; a<result->m_ItemCount; a++)
			if (m_Tags[a].cnt)
			{
				CRect rect(&m_Tags[a].rect);

				COLORREF color = m_ViewParameters.TagcloudUseColors ? m_Tags[a].color : text;

				if ((m_Tags[a].selected) || (HoverItem==(int)a))
				{
					if (hTheme)
					{
						const int StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
						UINT state = (m_Tags[a].selected ? 2 : 0) | (HoverItem==(int)a ? 1 : 0);
						theApp.zDrawThemeBackground(hTheme, dc.m_hDC, LVP_LISTITEM, StateIDs[state], rect, rect);
					}
					else
					{
						dc.FillSolidRect(rect, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE));
						color = GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT);
					}
				}
				else
					if (m_ViewParameters.TagcloudUseOpacity)
						color =((color & 0xFF)*m_Tags[a].alpha + (back & 0xFF)*(255-m_Tags[a].alpha))>>8 |
									((((color>>8) & 0xFF)*m_Tags[a].alpha + ((back>>8) & 0xFF)*(255-m_Tags[a].alpha)) & 0xFF00) |
									((((color>>16) & 0xFF)*m_Tags[a].alpha + ((back>>16) & 0xFF)*(255-m_Tags[a].alpha))<<8) & 0xFF0000;

				CFont* pOldFont = dc.SelectObject(GetFont(a));
				dc.SetTextColor(color);

				dc.DrawText(result->m_Items[a]->CoreAttributes.FileName, -1, rect, TextFormat | DT_CENTER | DT_VCENTER);

				if (((int)a==FocusItem) && (GetFocus()==this) && ((!hTheme) || (!m_Tags[a].selected)))
				{
					if (hTheme)
						rect.DeflateRect(1, 1);

					dc.SetTextColor(text);
					dc.DrawFocusRect(rect);
				}

				dc.SelectObject(pOldFont);
			}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CTagcloudView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CTagcloudView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

void CTagcloudView::OnSysColorChange()
{
	Invalidate();
}
