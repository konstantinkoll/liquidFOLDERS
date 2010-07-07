
// CTagcloudView.cpp: Implementierung der Klasse CTagcloudView
//

#include "stdafx.h"
#include "CTagcloudView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CTagcloudView
//

CTagcloudView::CTagcloudView()
{
	m_Tags = NULL;
	hTheme = NULL;

	CString face = theApp.GetDefaultFontFace();
	for (int a=0; a<24; a++)
		m_Fonts[a].CreateFont(-(a*2+10), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, a>=4 ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, face);

}

CTagcloudView::~CTagcloudView()
{
	if (m_Tags)
		delete[] m_Tags;
}

void CTagcloudView::Create(CWnd* _pParentWnd, LFSearchResult* _result)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewTagcloud);
}

void CTagcloudView::SetSearchResult(LFSearchResult* _result)
{
	if (m_Tags)
	{
		delete[] m_Tags;
		m_Tags = NULL;
	}

	if (_result)
		if (_result->m_ItemCount)
		{
			m_Tags = new Tag[_result->m_ItemCount];

			int minimum = -1;
			int maximum = -1;

			// Find items
			for (UINT a=0; a<_result->m_ItemCount; a++)
			{
				ZeroMemory(&m_Tags[a], sizeof(Tag));

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
				{
					m_Tags[a].alpha = 15+(240*(m_Tags[a].cnt-minimum))/delta;
					m_Tags[a].fontsize = (20*(m_Tags[a].cnt-minimum))/delta;
				}

		}

	Invalidate();
}

void CTagcloudView::SelectItem(int n, BOOL select, BOOL InternalCall)
{
	if (m_Tags)
	{
		m_Tags[n].selected = select;

		if (!InternalCall)
		{
			//UpdateScene(TRUE);
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
/*	if ((!m_Locations) || (!result) || (!m_ViewParameters.GlobeShowBubbles))
		return -1;

	int res = -1;
	float alpha = 0.0f;
	for (UINT a=0; a<result->m_ItemCount; a++)
	{
		if (m_Locations[a].cnt)
			if ((m_Locations[a].alpha>0.1f) && ((m_Locations[a].alpha>alpha-0.05f) || (m_Locations[a].alpha>0.75f)))
				if ((point.x>=m_Locations[a].screenlabel[0]) &&
					(point.x<m_Locations[a].screenlabel[2]) &&
					(point.y>=m_Locations[a].screenlabel[1]) &&
					(point.y<m_Locations[a].screenlabel[3]))
				{
					res = a;
					alpha = m_Locations[a].alpha;
				}
	}

	return res;*/
	return -1;
}


BEGIN_MESSAGE_MAP(CTagcloudView, CFileView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CTagcloudView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((theApp.m_ThemeLibLoaded) && (theApp.osInfo.dwMajorVersion>=6))
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

LRESULT CTagcloudView::OnThemeChanged()
{
	if ((theApp.m_ThemeLibLoaded) && (theApp.osInfo.dwMajorVersion>=6))
	{
		if (hTheme)
			theApp.zCloseThemeData(hTheme);

		hTheme = theApp.zOpenThemeData(m_hWnd, VSCLASS_LISTVIEW);
	}

	return TRUE;
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
				dc.SelectObject(m_Fonts[m_Tags[a].fontsize]);
				dc.TextOut(10, a*20, result->m_Items[a]->CoreAttributes.FileName);
			}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
