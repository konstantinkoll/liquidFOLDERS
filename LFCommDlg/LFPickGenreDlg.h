
// LFPickGenreDlg.h: Schnittstelle der Klasse LFPickGenreDlg
//

#pragma once
#include "LFCore.h"
#include "CFrontstageItemView.h"
#include "CInspectorGrid.h"


// CGenreList
//

#define GENREBUFFERSIZE     256

struct GenreItemData
{
	ItemData Hdr;
	LPCMUSICGENRE lpcMusicGenre;
	UINT GenreID;
	LPCWSTR pDescription;
	UINT FileCount;
	BOOL FirstInCategory;
};

class CGenreList sealed : public CFrontstageItemView
{
public:
	CGenreList();

	void SetGenres(ATTRIBUTE Attr, LFSearchResult* pSearchResult);
	void SelectGenre(UINT GenreID);
	UINT GetSelectedGenre() const;

protected:
	virtual void AdjustLayout();
	virtual void ShowTooltip(const CPoint& point);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

private:
	void AddItemCategory(LPCMUSICGENRE lpcMusicGenre);
	GenreItemData* GetGenreItemData(INT Index) const;
	void AddMusicGenreCategory(UINT IconID);
	void AddItem(LPCMUSICGENRE lpcMusicGenre, UINT GenreID, BOOL FirstInCategory=TRUE);

	WCHAR m_Description[GENREBUFFERSIZE][256];
	UINT m_FileCount[GENREBUFFERSIZE];
};

inline GenreItemData* CGenreList::GetGenreItemData(INT Index) const
{
	return (GenreItemData*)GetItemData(Index);
}

inline void CGenreList::AddItemCategory(LPCMUSICGENRE lpcMusicGenre)
{
	CFrontstageItemView::AddItemCategory(lpcMusicGenre->Name, L"", lpcMusicGenre->IconID);
}


// LFPickGenreDlg
//

class LFPickGenreDlg : public CAttributePickDlg
{
public:
	LFPickGenreDlg(ATTRIBUTE Attr, ITEMCONTEXT Context, UINT Genre, CWnd* pParentWnd=NULL);

	UINT m_Genre;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CGenreList m_wndGenreList;

private:
	BOOL m_SelectGenre;
};
