
// LFEditGenreDlg.cpp: Implementierung der Klasse LFEditGenreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CGenreList
//

CGenreList::CGenreList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(GenreItemData))
{
}

void CGenreList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	const GenreItemData* pData = GetGenreItemData(m_HoverItem);

	LFGetApp()->ShowTooltip(this, point, pData->pMusicGenre->Name, pData->pDescription);
}

void CGenreList::AdjustLayout()
{
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	// Assume this list is so tall that a scrollbar is always neccessary
	m_ScrollWidth = rectLayout.Width()-GetSystemMetrics(SM_CXVSCROLL)-ITEMVIEWMARGIN;
	
	INT CategoriesHeight = 0;
	INT ItemsHeight = 0;

	UINT CurrentCategory = 0;

	for (INT a=0; a<m_ItemCount; a++)
	{
		GenreItemData* pData = GetGenreItemData(a);
		ASSERT(pData->Hdr.Valid);

		if (pData->FirstInCategory)
		{
			ASSERT(CurrentCategory<m_ItemCategories.m_ItemCount);

			const LPRECT lpRect = &m_ItemCategories[CurrentCategory++].Rect;
			lpRect->left = BACKSTAGEBORDER;
			lpRect->right = m_ScrollWidth;
			lpRect->bottom = (lpRect->top=max(CategoriesHeight, ItemsHeight)+BACKSTAGEBORDER)+2*LFCATEGORYPADDING+m_LargeFontHeight+2*LFCATEGORYPADDING;

			ItemsHeight = lpRect->bottom+BACKSTAGEBORDER/2;
			CategoriesHeight = ItemsHeight+128-2*ITEMVIEWICONPADDING;
		}

		pData->Hdr.Row = a;
		pData->Hdr.Rect.left = 2*BACKSTAGEBORDER+128-2*ITEMVIEWICONPADDING;
		pData->Hdr.Rect.right = m_ScrollWidth;
		pData->Hdr.Rect.bottom = (pData->Hdr.Rect.top=ItemsHeight)+m_ItemHeight;

		ItemsHeight = pData->Hdr.Rect.bottom-1;
	}

	m_ScrollHeight = max(CategoriesHeight, ItemsHeight+1)+ITEMVIEWMARGIN;

	CFrontstageScroller::AdjustLayout();
}

void CGenreList::AddItem(const LFMusicGenre* pMusicGenre, UINT GenreID, BOOL FirstInCategory)
{
	ASSERT(pMusicGenre);

	// Hide some non-primary genres like "Negerpunk"
	if (!pMusicGenre->Show)
	{
		ASSERT(!pMusicGenre->Primary);

		return;
	}

	GenreItemData Data;

	Data.pMusicGenre = pMusicGenre;
	Data.GenreID = GenreID;
	Data.pDescription = m_Description[GenreID];
	Data.FileCount = m_FileCount[GenreID];
	Data.FirstInCategory = FirstInCategory;

	CFrontstageItemView::AddItem(&Data);
}

void CGenreList::AddMusicGenreCategory(UINT IconID)
{
	const LFMusicGenre* pMusicGenre;

	INT Index = LFID3GetNextMusicGenreByIcon(IconID, 0, &pMusicGenre);			// Skip genre 0
	while (Index!=-1)
	{
		// Skip primary genre for this icon!
		if (!pMusicGenre->Primary)
			AddItem(pMusicGenre, Index, FALSE);

		Index = LFID3GetNextMusicGenreByIcon(IconID, Index, &pMusicGenre);
	}
}

void CGenreList::SetGenres(const STOREID& StoreID)
{
	// Gather statistics
	ZeroMemory(m_Description, sizeof(m_Description));
	ZeroMemory(m_FileCount, sizeof(m_FileCount));

	// Filter
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Query.Context = LFContextMusic;
	pFilter->Query.StoreID = StoreID;

	// Query
	CWaitCursor WaitCursor;

	LFSearchResult* pRawFiles = LFQuery(pFilter);
	LFSearchResult* pCookedFiles = LFGroupSearchResult(pRawFiles, LFAttrGenre, FALSE, TRUE, pFilter);
	LFFreeFilter(pFilter);

	for (UINT a=0; a<pCookedFiles->m_ItemCount; a++)
	{
		LFVariantData VData;
		LFGetAttributeVariantDataEx((*pCookedFiles)[a], LFAttrGenre, VData);

		if (!LFIsNullVariantData(VData) && (VData.Genre<GENREBUFFERSIZE))
		{
			wcscpy_s(m_Description[VData.Genre], 256, (*pCookedFiles)[a]->Description);
			m_FileCount[VData.Genre] = (*pCookedFiles)[a]->AggregateCount;
		}
	}

	LFFreeSearchResult(pCookedFiles);
	LFFreeSearchResult(pRawFiles);

	// Add music genres
	SetItemCount(GENREBUFFERSIZE, FALSE);

	const LFMusicGenre* pOtherPrimary = NULL;
	INT OtherPrimaryIndex = 0;

	const LFMusicGenre* pMusicGenre;

	INT Index = LFID3GetNextMusicGenre(0, &pMusicGenre);			// Skip genre 0
	while (Index!=-1)
	{
		if (pMusicGenre->Primary)
			if (pMusicGenre->IconID==IDI_FLD_DEFAULTGENRE)
			{
				pOtherPrimary = pMusicGenre;
				OtherPrimaryIndex = Index;
			}
			else
			{
				AddItemCategory(pMusicGenre);
				AddItem(pMusicGenre, Index);
				AddMusicGenreCategory(pMusicGenre->IconID);
			}

		Index = LFID3GetNextMusicGenre(Index, &pMusicGenre);
	}

	// Add "Other" category
	if (pOtherPrimary)
	{
		AddItemCategory(pOtherPrimary);
		AddItem(pOtherPrimary, OtherPrimaryIndex);
		AddMusicGenreCategory(pOtherPrimary->IconID);
	}

	LastItem();
	AdjustLayout();
}

void CGenreList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const GenreItemData* pData = GetGenreItemData(Index);

	CRect rect(rectItem);
	rect.DeflateRect(2*ITEMCELLPADDING, ITEMCELLPADDING);

	if (pData->FileCount)
		rect.right -= m_FileCountWidth+ITEMCELLPADDING;

	// Name
	dc.DrawText(pData->pMusicGenre->Name, -1, rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_VCENTER);

	// Count
	if (pData->FileCount)
	{
		rect.left = rect.right+ITEMCELLPADDING;
		rect.right += m_FileCountWidth;

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallFont);

		SetLightTextColor(dc, Index, Themed);
		dc.DrawText(CBackstageSidebar::FormatCount(pData->FileCount), rect, DT_SINGLELINE | DT_NOPREFIX | DT_RIGHT | DT_VCENTER);

		dc.SelectObject(pOldFont);
	}
}

void CGenreList::SelectGenre(UINT GenreID)
{
	for (INT a=0; a<m_ItemCount; a++)
		if (GetGenreItemData(a)->GenreID==GenreID)
		{
			SetFocusItem(a);
			EnsureVisible(a);

			break;
		}
}

UINT CGenreList::GetSelectedGenre() const
{
	const INT Index = GetSelectedItem();

	return (Index>=0) ? GetGenreItemData(Index)->GenreID : 0;
}


BEGIN_MESSAGE_MAP(CGenreList, CFrontstageItemView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

INT CGenreList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageItemView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Item
	SetItemHeight(0, 1, ITEMCELLPADDING);
	m_FileCountWidth = LFGetApp()->m_SmallFont.GetTextExtent(_T("000W")).cx+2*ITEMCELLPADDING;

	return 0;
}


// LFEditGenreDlg
//

LFEditGenreDlg::LFEditGenreDlg(UINT Genre, const STOREID& StoreID, CWnd* pParentWnd)
	: LFDialog(IDD_EDITGENRE, pParentWnd)
{
	m_Genre = Genre;
	m_StoreID = StoreID;

	m_SelectGenre = TRUE;
}

void LFEditGenreDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
		m_Genre = m_wndGenreList.GetSelectedGenre();
}

void LFEditGenreDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	if (IsWindow(m_wndGenreList))
	{
		m_wndGenreList.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), m_BottomDivider-rectLayout.top, nFlags);

		if (m_SelectGenre)
		{
			m_wndGenreList.SelectGenre(m_Genre);
			m_SelectGenre = FALSE;
		}
	}
}

BOOL LFEditGenreDlg::InitDialog()
{
	m_wndGenreList.Create(this, IDC_GENRELIST);
	m_wndGenreList.SetGenres(m_StoreID);

	m_wndGenreList.SetFocus();

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFEditGenreDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

void LFEditGenreDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 360);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}
