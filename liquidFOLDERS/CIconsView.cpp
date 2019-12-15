
// CIconsView.cpp: Implementierung der Klasse CIconsView
//

#include "stdafx.h"
#include "CIconsView.h"
#include "liquidFOLDERS.h"


// CIconsView
//

#define CAPACITYBARHEIGHT     10

CIconsView::CIconsView()
	: CFileView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FRONTSTAGE_ENABLELABELEDIT | FRONTSTAGE_ENABLEEDITONHOVER | FF_ENABLEFOLDERTOOLTIPS)
{
}

void CIconsView::SetViewSettings(BOOL UpdateSearchResultPending)
{
	// Commit settings
	if (!UpdateSearchResultPending)
		Invalidate();
}

void CIconsView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	ValidateAllItems();
}


// Layouts

void CIconsView::AdjustLayout()
{
	// Item width must be odd for proper alignment of jumbo core icons
	AdjustLayoutGrid(CSize(max(127, m_DefaultFontHeight*8+1)+2*ITEMVIEWPADDING, 128+m_DefaultFontHeight*2+2*ITEMVIEWPADDING+ITEMVIEWPADDING/2),
		FALSE, BACKSTAGEBORDER);
}


// Draw support

void CIconsView::DrawCapacityBar(Graphics& g, const CRect& rect, const LFStoreDescriptor& StoreDescriptor)
{
	// Calculate bar sizes
	INT64 szTotal = StoreDescriptor.TotalNumberOfBytes.QuadPart;
	const INT64 szFree = StoreDescriptor.FreeBytesAvailable.QuadPart;
	const INT64 szStore = StoreDescriptor.Statistics.FileSize[LFContextAllFiles];
	const INT64 szTrash = StoreDescriptor.Statistics.FileSize[LFContextTrash];

	INT64 szOccupied = szTotal-szFree-szStore;

	ASSERT(szOccupied>=0);
	if (szOccupied<0)
	{
		szTotal -= szOccupied;
		szOccupied = 0;
	}

	// Draw bars
	const REAL Width = (REAL)rect.Width();

	const INT WidthStore = (INT)((REAL)(szOccupied+szTrash+szStore)*Width/(REAL)szTotal+0.5);
	const REAL WidthTrash = (REAL)(szOccupied+szTrash)*Width/(REAL)szTotal;
	const REAL WidthOccupied = (REAL)szOccupied*Width/(REAL)szTotal;

	g.ResetClip();
	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	SolidBrush brush(Color(0xFF25A9FF));
	g.FillRectangle(&brush, (REAL)rect.left, (REAL)rect.top, (REAL)WidthStore, (REAL)rect.Height());

	if (szTrash)
	{
		brush.SetColor(Color(0xFF303038));
		g.FillRectangle(&brush, (REAL)rect.left, (REAL)rect.top, WidthTrash, (REAL)rect.Height());
	}

	brush.SetColor(Color(0xFF1051D1));
	g.FillRectangle(&brush, (REAL)rect.left, (REAL)rect.top, WidthOccupied, (REAL)rect.Height());

	// Clipping
	g.SetClip(Rect(0, 0, WidthStore, rect.Height()));
}

void CIconsView::DrawCapacity(CDC& dc, Graphics& g, CRect rectCapacity, const LFStoreDescriptor& StoreDescriptor, BOOL Themed) const
{
	ASSERT(rectCapacity.Height()>=CAPACITYBARHEIGHT);

	// Draw bar background
	if (Themed)
	{
		// Border and background
		DrawMilledRectangle(g, rectCapacity, FALSE);
		rectCapacity.DeflateRect(2, 2);
		rectCapacity.bottom++;

		// Bar
		const INT Width = rectCapacity.Width();
		const INT Height = rectCapacity.Height();
		Bitmap bMemory(Width, Height);

		CRect rectBars(0, 0, Width, Height);

		Graphics* gBars = Graphics::FromImage(&bMemory);
		DrawCapacityBar(*gBars, rectBars, StoreDescriptor);

		// Decorate bar
		TextureBrush brushStripes(theApp.GetCachedResourceImage(IDB_BACKGROUND_CAPACITYBAR));
		gBars->FillRectangle(&brushStripes, 0, 0, Width, Height);
		gBars->FillRectangle(&brushStripes, 0, 0, Width, Height);

		const INT Bevel = Height/3;
		LinearGradientBrush brushGradient1(Point(0, Bevel), Point(0, Height), Color(0x00000000), Color(0x40000000));
		gBars->FillRectangle(&brushGradient1, 0, Bevel, Width, Height-Bevel);

		GraphicsPath path;
		CreateRoundTop(rectBars, 3, path);

		Pen pen(Color(0x60FFFFFF));
		gBars->DrawPath(&pen, &path);

		Matrix m;
		m.Translate(0.0f, 1.0f);
		path.Transform(&m);

		pen.SetColor(Color(0x40FFFFFF));
		gBars->DrawPath(&pen, &path);

		CreateRoundRectangle(rectBars, 3, path);

		LinearGradientBrush brushGradient2(Point(0, 0), Point(0, Height), Color(0x00000000), Color(0x40000000));
		pen.SetBrush(&brushGradient2);
		gBars->DrawPath(&pen, &path);

		// Draw bar
		TextureBrush brushTexture(&bMemory, WrapModeTile);
		brushTexture.TranslateTransform((REAL)rectCapacity.left, (REAL)rectCapacity.top);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		CreateRoundRectangle(rectCapacity, 3, path);
		g.FillPath(&brushTexture, &path);
	}
	else
	{
		// Border
		dc.Draw3dRect(rectCapacity, GetSysColor(COLOR_WINDOWTEXT), GetSysColor(COLOR_WINDOWTEXT));

		// Background
		rectCapacity.DeflateRect(1, 1);
		dc.FillSolidRect(rectCapacity, 0xE7E7E7);

		// Bar
		DrawCapacityBar(g, rectCapacity, StoreDescriptor);
	}
}

void CIconsView::DrawWrapLabel(CDC& dc, Graphics& g, const CRect& rectLabel, LFItemDescriptor* pItemDescriptor, BOOL Themed, UINT MaxLineCount) const
{
	// Make width one pixel larger due to ClearType
	const INT MaxLineWidth = rectLabel.Width()+1;

	// Label and with of color dots
	CString strLabel = GetItemLabel(pItemDescriptor);
	INT ColorDotWidth = GetColorDotWidth(pItemDescriptor);

	if (LFIsFolder(pItemDescriptor) && LFHasSubcaption(pItemDescriptor))
		strLabel.Truncate(pItemDescriptor->FolderMainCaptionCount);

	// We only care for the height of rectLine here
	CRect rectLine(rectLabel);
	rectLine.bottom = rectLine.top+m_DefaultFontHeight;

	// Iterate the lines
	UINT Line = 0;

	while (Line<MaxLineCount)
	{
		CString strLine;
		INT LineWidth = 0;
		BOOL Break;

		do
		{
			// Find delimiter
			INT Pos = strLabel.Find(L' ');

			// If none is found, use end of string
			if (Pos==-1)
				Pos = strLabel.GetLength();

			// Extract next token
			const CString strToken = strLabel.Left(Pos);

			// Calculate new line width, assuming the token would be concatenated
			const INT NewLineWidth = ColorDotWidth+dc.GetTextExtent(strLine+strToken).cx;

			// Do we add the token?
			if (((Break=(NewLineWidth>MaxLineWidth))==FALSE) || strLine.IsEmpty())
			{
				// Yes, so we have a new line width...
				LineWidth = NewLineWidth;
				strLine += strToken+_T(" ");

				// ...and consume the token! Also remove all extra spaces on the left
				strLabel.Delete(0, Pos+1);
				strLabel = strLabel.TrimLeft();
			}
		}
		while (!Break && !strLabel.IsEmpty());

		// There might me trailing spaces
		strLine.TrimRight();

		// Calculate left and right bounds of line
		rectLine.left = (rectLabel.left+rectLabel.right-min(MaxLineWidth, LineWidth))/2;
		rectLine.right = min(rectLabel.right, rectLine.left+LineWidth);

		// Draw color dots on first line
		if (ColorDotWidth)
		{
			DrawColorDots(dc, rectLine, pItemDescriptor, m_DefaultFontHeight);
			ColorDotWidth = 0;
		}

		// Draw text
		dc.DrawText(strLine, rectLine, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_TOP | DT_SINGLELINE);

		// Move rectLine one line down
		rectLine.OffsetRect(0, m_DefaultFontHeight);

		Line++;

		if (strLabel.IsEmpty())
			break;
	}

	// Hint
	if (Line<MaxLineCount)
		switch (LFGetItemType(pItemDescriptor))
		{
		case LFTypeStore:
			if (m_GlobalViewSettings.IconsShowCapacity)
				DrawCapacity(dc, g, CRect(rectLabel.left, rectLabel.bottom-max(m_SmallFontHeight, CAPACITYBARHEIGHT), rectLabel.right, rectLabel.bottom), pItemDescriptor->StoreDescriptor, Themed);

			break;

		case LFTypeFolder:
			LPCWSTR pHint = LFHasSubcaption(pItemDescriptor) ? LFGetSubcaption(pItemDescriptor) : pItemDescriptor->Description;
			ASSERT(pHint[0]);

			CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);

			SetLightTextColor(dc, pItemDescriptor, Themed);
			dc.DrawText(pHint, -1, CRect(rectLabel.left, rectLabel.bottom-m_SmallFontHeight, rectLabel.right, rectLabel.bottom), DT_CENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

			dc.SelectObject(pOldFont);

			break;
		}
}


// Drawing

void CIconsView::DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	// Label
	if (!IsEditing() || (Index!=m_EditItem))
	{
		CRect rectLabel(rectItem);
		rectLabel.DeflateRect(Themed ? ITEMVIEWPADDING+1 : ITEMVIEWPADDING, ITEMVIEWPADDING);
		rectLabel.top += 128+ITEMVIEWPADDING/2;

		DrawWrapLabel(dc, g, rectLabel, pItemDescriptor, Themed);
	}

	// Icon
	DrawJumboIcon(dc, g, CPoint((rectItem->left+rectItem->right-128)/2, rectItem->top+ITEMVIEWPADDING), pItemDescriptor);
}


// Label edit

RECT CIconsView::GetLabelRect() const
{
	ASSERT(m_EditItem>=0);
	ASSERT(m_EditItem<m_ItemCount);

	RECT rect = GetItemRect(m_EditItem);

	rect.bottom = (rect.top+=128+ITEMVIEWPADDING+ITEMVIEWPADDING/2-2)+m_DefaultFontHeight+4;
	rect.left += ITEMVIEWPADDING/2;
	rect.right -= ITEMVIEWPADDING/2;

	return rect;
}


BEGIN_MESSAGE_MAP(CIconsView, CFileView)
	ON_COMMAND(IDM_ICONS_SHOWCAPACITY, OnShowCapacity)
	ON_UPDATE_COMMAND_UI(IDM_ICONS_SHOWCAPACITY, OnUpdateCommands)
END_MESSAGE_MAP()

void CIconsView::OnShowCapacity()
{
	p_GlobalViewSettings->IconsShowCapacity = !p_GlobalViewSettings->IconsShowCapacity;

	theApp.UpdateViewSettings(LFContextStores, LFViewIcons);
}

void CIconsView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_ICONS_SHOWCAPACITY:
		bEnable &= (m_Context==LFContextStores);
		pCmdUI->SetCheck(m_GlobalViewSettings.IconsShowCapacity);

		break;
	}

	pCmdUI->Enable(bEnable);
}
