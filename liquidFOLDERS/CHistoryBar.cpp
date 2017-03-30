
// CHistoryBar.cpp: Implementierung der Klasse CHistoryBar
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// Breadcrumbs
//

void AddBreadcrumbItem(BreadcrumbItem** ppBreadcrumbItem, LFFilter* pFilter, FVPersistentData& Data)
{
	BreadcrumbItem* pNewItem = new BreadcrumbItem;
	pNewItem->pNext = *ppBreadcrumbItem;
	pNewItem->pFilter = pFilter;
	pNewItem->Data = Data;

	*ppBreadcrumbItem = pNewItem;
}

void ConsumeBreadcrumbItem(BreadcrumbItem** ppBreadcrumbItem, LFFilter** ppFilter, FVPersistentData* pPersistentData)
{
	*ppFilter = NULL;
	ZeroMemory(pPersistentData, sizeof(FVPersistentData));

	if (*ppBreadcrumbItem)
	{
		*ppFilter = (*ppBreadcrumbItem)->pFilter;
		*pPersistentData = (*ppBreadcrumbItem)->Data;

		BreadcrumbItem* pVictim = *ppBreadcrumbItem;
		*ppBreadcrumbItem = (*ppBreadcrumbItem)->pNext;
		delete pVictim;
	}
}

void DeleteBreadcrumbs(BreadcrumbItem** ppBreadcrumbItem)
{
	while (*ppBreadcrumbItem)
	{
		BreadcrumbItem* pVictim = *ppBreadcrumbItem;
		*ppBreadcrumbItem = (*ppBreadcrumbItem)->pNext;

		LFFreeFilter(pVictim->pFilter);
		delete pVictim;
	}
}


// CHistoryBar
//

#define MARGIN     4

BOOL CHistoryBar::Create(CWnd* pParentWnd, UINT nID)
{
	return CBackstageBar::Create(pParentWnd, nID, theApp.m_DefaultFont.GetFontHeight()+1, TRUE);
}

void CHistoryBar::AddItem(const LFFilter* pFilter, CDC& dc)
{
	BarItem Item;
	ZeroMemory(&Item, sizeof(Item));

	wcscpy_s(Item.Name, 256, pFilter->ResultName);
	Item.IconID = -1;
	Item.PreferredWidth = dc.GetTextExtent(Item.Name, (INT)wcslen(Item.Name)).cx+2*MARGIN;
	Item.Enabled = TRUE;

	m_BarItems.AddItem(Item);
}

void CHistoryBar::SetHistory(const LFFilter* pFilter, BreadcrumbItem* pBreadcrumbItem)
{
	Reset();

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	AddItem(pFilter, dc);
	while (pBreadcrumbItem)
	{
		AddItem(pBreadcrumbItem->pFilter, dc);
		pBreadcrumbItem = pBreadcrumbItem->pNext;
	}

	dc.SelectObject(pOldFont);

	AdjustLayout();
}

void CHistoryBar::DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT /*State*/, BOOL Themed)
{
	// Arrow
	if (Index)
	{
		CPen pen(PS_SOLID, 1, Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW));
		CPen* pOldPen = dc.SelectObject(&pen);

		CRect rectArrow(rectItem.right, rectItem.top, rectItem.right+m_Spacer, rectItem.bottom);

		INT cy = rectArrow.Height()/4;
		INT cx = (rectArrow.Width()-cy)/2;
		INT cc = (rectArrow.top+rectArrow.bottom)/2;
		for (INT y=0; y<cy; y++)
		{
			dc.MoveTo(rectArrow.left+cx, cc+y);
			dc.LineTo(rectArrow.left+cx+cy-y, cc+y);
			dc.MoveTo(rectArrow.left+cx, cc-y);
			dc.LineTo(rectArrow.left+cx+cy-y, cc-y);
		}

		dc.SelectObject(pOldPen);
	}

	// Label
	rectItem.DeflateRect(MARGIN, 0);
	dc.DrawText(m_BarItems[Index].Name, -1, rectItem, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
}

void CHistoryBar::OnClickButton(INT Index) const
{
	GetOwner()->PostMessage(Index ? WM_NAVIGATEBACK : WM_RELOAD, (WPARAM)Index);
}
