
// LFEditGenreDlg.h: Schnittstelle der Klasse LFEditGenreDlg
//

#pragma once
#include "LFCore.h"
#include "CGenreList.h"


// LFEditGenreDlg
//

#define GENREBUFFERSIZE     256

class LFEditGenreDlg : public LFDialog
{
public:
	LFEditGenreDlg(UINT Genre, LPCSTR pStoreID, CWnd* pParentWnd=NULL);

	UINT GetSelectedGenre() const;

	UINT m_Genre;

protected:
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	CGenreList m_wndGenreList;

private:
	void AddItem(LFMusicGenre* pMusicGenre, INT Index);
	void AddCategory(INT IconID);

	CHAR m_StoreID[LFKeySize];
	UINT m_FileCount[GENREBUFFERSIZE];
	WCHAR m_Description[GENREBUFFERSIZE][64];
};

inline UINT LFEditGenreDlg::GetSelectedGenre() const
{
	return m_wndGenreList.GetSelectedGenre();
}
