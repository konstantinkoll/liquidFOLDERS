
// LFEditGenreDlg.cpp: Implementierung der Klasse LFEditGenreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFEditGenreDlg
//

LFEditGenreDlg::LFEditGenreDlg(UINT Genre, LPCSTR pStoreID, CWnd* pParentWnd)
	: LFDialog(IDD_EDITGENRE, pParentWnd)
{
	ASSERT(pStoreID);

	m_Genre = Genre;
	strcpy_s(m_StoreID, LFKeySize, pStoreID);
}

void LFEditGenreDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	if (IsWindow(m_wndGenreList))
		m_wndGenreList.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), m_BottomDivider-rectLayout.top, nFlags);
}

void LFEditGenreDlg::AddItem(const LFMusicGenre* pMusicGenre, INT Index)
{
	m_wndGenreList.AddItem(pMusicGenre, Index, Index<GENREBUFFERSIZE ? m_FileCount[Index] : 0, Index<GENREBUFFERSIZE ? m_Description[Index] : 0);
}

void LFEditGenreDlg::AddCategory(INT IconID)
{
	const LFMusicGenre* pMusicGenre;

	INT Index = LFID3GetNextMusicGenreByIcon(IconID, 0, &pMusicGenre);			// Skip genre 0
	while (Index!=-1)
	{
		if (!pMusicGenre->Primary)
			AddItem(pMusicGenre, Index);

		Index = LFID3GetNextMusicGenreByIcon(IconID, Index, &pMusicGenre);
	}
}

BOOL LFEditGenreDlg::InitDialog()
{
	m_wndGenreList.Create(this, IDC_GENRELIST);

	// Gather statistics
	CWaitCursor csr;

	LFFilter* pFilter = LFAllocFilter();
	pFilter->QueryContext = LFContextAudio;

	if (!m_StoreID[0])
	{
		pFilter->Mode = LFFilterModeSearch;
	}
	else
	{
		pFilter->Mode = LFFilterModeDirectoryTree;
		strcpy_s(pFilter->StoreID, LFKeySize, m_StoreID);
	}

	LFSearchResult* pRawFiles = LFQuery(pFilter);
	LFSearchResult* pCookedFiles = LFGroupSearchResult(pRawFiles, LFAttrGenre, FALSE, TRUE, pFilter);
	LFFreeFilter(pFilter);

	ZeroMemory(m_FileCount, sizeof(m_FileCount));
	ZeroMemory(m_Description, sizeof(m_Description));

	for (UINT a=0; a<pCookedFiles->m_ItemCount; a++)
	{
		LFVariantData Property;
		LFGetAttributeVariantDataEx((*pCookedFiles)[a], LFAttrGenre, Property);

		if (!LFIsNullVariantData(Property))
		{
			ASSERT(Property.UINT32<256);

			m_FileCount[Property.UINT32] = (*pCookedFiles)[a]->AggregateCount;
			wcsncpy_s(m_Description[Property.UINT32], 256, (*pCookedFiles)[a]->Description, _TRUNCATE);
		}
	}

	LFFreeSearchResult(pCookedFiles);
	LFFreeSearchResult(pRawFiles);

	// Add music genres
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
				m_wndGenreList.AddCategory(pMusicGenre);
				AddItem(pMusicGenre, Index);

				AddCategory(pMusicGenre->IconID);
			}

		Index = LFID3GetNextMusicGenre(Index, &pMusicGenre);
	}

	// Add "Other" section
	if (pOtherPrimary)
	{
		m_wndGenreList.AddCategory(pOtherPrimary);
		AddItem(pOtherPrimary, OtherPrimaryIndex);

		AddCategory(pOtherPrimary->IconID);
	}

	m_wndGenreList.SelectGenre(m_Genre);
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
