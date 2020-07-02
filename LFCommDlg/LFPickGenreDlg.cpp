
// LFPickGenreDlg.cpp: Implementierung der Klasse LFPickGenreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFPickGenreDlg.h"


// CGenreList
//

CGenreList::CGenreList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(GenreItemData))
{
	// Item
	SetItemHeight(0, 1, ITEMCELLPADDINGY);
}


// Layouts

void CGenreList::AdjustLayout()
{
	// Item separator
	m_DrawItemSeparator = FALSE;

	// Layout rect
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

		if (pData->FirstInCategory)
			ItemsHeight += m_ItemHeight/2;
	}

	m_ScrollHeight = max(CategoriesHeight, ItemsHeight+1)+ITEMVIEWMARGIN;

	CFrontstageScroller::AdjustLayout();
}


// Item data

void CGenreList::AddItem(LPCMUSICGENRE lpcMusicGenre, UINT GenreID, BOOL FirstInCategory)
{
	ASSERT(lpcMusicGenre);

	// Hide some non-primary genres like "Negerpunk"
	if (!lpcMusicGenre->Show)
	{
		ASSERT(!lpcMusicGenre->Primary);

		return;
	}

	GenreItemData Data;

	Data.lpcMusicGenre = lpcMusicGenre;
	Data.GenreID = GenreID;
	Data.pDescription = m_Description[GenreID];
	Data.FileCount = m_FileCount[GenreID];
	Data.FirstInCategory = FirstInCategory;

	CFrontstageItemView::AddItem(&Data);
}

void CGenreList::AddMusicGenreCategory(UINT IconID)
{
	LPCMUSICGENRE lpcMusicGenre;

	INT Index = LFID3GetNextMusicGenreByIcon(IconID, 0, lpcMusicGenre);			// Skip genre 0
	while (Index!=-1)
	{
		// Skip primary genre for this icon!
		if (!lpcMusicGenre->Primary)
			AddItem(lpcMusicGenre, Index, FALSE);

		Index = LFID3GetNextMusicGenreByIcon(IconID, Index, lpcMusicGenre);
	}
}

void CGenreList::SetGenres(ATTRIBUTE Attr, LFSearchResult* pSearchResult)
{
	// Gather statistics
	ZeroMemory(m_Description, sizeof(m_Description));
	ZeroMemory(m_FileCount, sizeof(m_FileCount));
	
	// Aggregate
	for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
	{
		LFVariantData VData;
		LFGetAttributeVariantDataEx((*pSearchResult)[a], Attr, VData);

		if (!LFIsNullVariantData(VData) && (VData.Genre<GENREBUFFERSIZE))
		{
			wcscpy_s(m_Description[VData.Genre], 256, (*pSearchResult)[a]->Description);
			m_FileCount[VData.Genre] = (*pSearchResult)[a]->AggregateCount;
		}
	}

	// Free search result
	LFFreeSearchResult(pSearchResult);

	// Add music genres
	SetItemCount(GENREBUFFERSIZE, FALSE);

	LPCMUSICGENRE lpcOtherPrimary = NULL;
	INT OtherPrimaryIndex = 0;

	LPCMUSICGENRE lpcMusicGenre;

	INT Index = LFID3GetNextMusicGenre(0, lpcMusicGenre);			// Skip genre 0
	while (Index!=-1)
	{
		if (lpcMusicGenre->Primary)
			if (lpcMusicGenre->IconID==IDI_FLD_DEFAULTGENRE)
			{
				lpcOtherPrimary = lpcMusicGenre;
				OtherPrimaryIndex = Index;
			}
			else
			{
				AddItemCategory(lpcMusicGenre);
				AddItem(lpcMusicGenre, Index);
				AddMusicGenreCategory(lpcMusicGenre->IconID);
			}

		Index = LFID3GetNextMusicGenre(Index, lpcMusicGenre);
	}

	// Add "Other" category
	if (lpcOtherPrimary)
	{
		AddItemCategory(lpcOtherPrimary);
		AddItem(lpcOtherPrimary, OtherPrimaryIndex);
		AddMusicGenreCategory(lpcOtherPrimary->IconID);
	}

	// Finish
	LastItem();
	AdjustLayout();
}


// Item handling

void CGenreList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	const GenreItemData* pData = GetGenreItemData(m_HoverItem);

	LFGetApp()->ShowTooltip(this, point, pData->lpcMusicGenre->Name, pData->pDescription);
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


// Item selection

UINT CGenreList::GetSelectedGenre() const
{
	const INT Index = GetSelectedItem();

	return (Index>=0) ? GetGenreItemData(Index)->GenreID : 0;
}


// Drawing

void CGenreList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const GenreItemData* pData = GetGenreItemData(Index);

	DrawCountItem(dc, rectItem, Index, Themed, pData->lpcMusicGenre->Name, pData->FileCount);
}


// LFPickGenreDlg
//

LFPickGenreDlg::LFPickGenreDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, UINT Genre, CWnd* pParentWnd)
	: CAttributePickDlg(Attr, Context, IDD_PICKGENRE, pParentWnd)
{
	ASSERT(LFGetApp()->m_Attributes[Attr].AttrProperties.Type==LFTypeGenre);

	m_Genre = Genre;
	m_SelectGenre = TRUE;
}

void LFPickGenreDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
		m_Genre = m_wndGenreList.GetSelectedGenre();
}

void LFPickGenreDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	CAttributePickDlg::AdjustLayout(rectLayout, nFlags);

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

BOOL LFPickGenreDlg::InitDialog()
{
	CAttributePickDlg::InitDialog();

	// Genre list
	m_wndGenreList.Create(this, IDC_GENRELIST);
	m_wndGenreList.SetGenres(m_Attr, RunQuery());

	m_wndGenreList.SetFocus();

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFPickGenreDlg, CAttributePickDlg)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(IVN_SELECTIONCHANGED, IDC_GENRELIST, OnSelectionChanged)
END_MESSAGE_MAP()

void LFPickGenreDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CAttributePickDlg::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 360);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

void LFPickGenreDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	GetDlgItem(IDOK)->EnableWindow(m_wndGenreList.GetSelectedGenre());

	*pResult = 0;
}
