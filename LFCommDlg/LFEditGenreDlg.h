
// LFEditGenreDlg.h: Schnittstelle der Klasse LFEditGenreDlg
//

#pragma once
#include "LFCore.h"
#include "CFrontstageItemView.h"


// CGenreList
//

#define GENREBUFFERSIZE     256

struct GenreItemData
{
	ItemData Hdr;
	const LFMusicGenre* pMusicGenre;
	UINT GenreID;
	LPCWSTR pDescription;
	UINT FileCount;
	BOOL FirstInCategory;
};

class CGenreList : public CFrontstageItemView
{
public:
	CGenreList();

	void SetGenres(const STOREID& StoreID);
	void SelectGenre(UINT GenreID);
	UINT GetSelectedGenre() const;

protected:
	virtual void AdjustLayout();
	virtual void ShowTooltip(const CPoint& point);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

private:
	GenreItemData* GetGenreItemData(INT Index) const;
	void AddItemCategory(const LFMusicGenre* pMusicGenre);
	void AddItem(const LFMusicGenre* pMusicGenre, UINT GenreID, BOOL FirstInCategory=TRUE);
	void AddMusicGenreCategory(UINT IconID);

	INT m_FileCountWidth;

	WCHAR m_Description[GENREBUFFERSIZE][256];
	UINT m_FileCount[GENREBUFFERSIZE];
};

inline GenreItemData* CGenreList::GetGenreItemData(INT Index) const
{
	return (GenreItemData*)GetItemData(Index);
}

inline void CGenreList::AddItemCategory(const LFMusicGenre* pMusicGenre)
{
	CFrontstageItemView::AddItemCategory(pMusicGenre->Name, L"", pMusicGenre->IconID);
}


// LFEditGenreDlg
//

class LFEditGenreDlg : public LFDialog
{
public:
	LFEditGenreDlg(UINT Genre, const STOREID& StoreID, CWnd* pParentWnd=NULL);

	UINT m_Genre;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	CGenreList m_wndGenreList;

private:
	STOREID m_StoreID;
	BOOL m_SelectGenre;
};
