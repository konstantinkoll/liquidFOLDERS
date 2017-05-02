
// CGridView.h: Schnittstelle der Klasse CGridView
//

#pragma once
#include "CFileView.h"


// CGridView
//

struct GridItemData
{
	FVItemData Hdr;
	INT Row;
	INT Column;
};

class CGridView : public CFileView
{
public:
	CGridView(SIZE_T DataSize=sizeof(GridItemData), UINT Flags=0);

protected:
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)=NULL;

	void ResetItemCategories();
	void Arrange(CSize szItem, INT Padding, CSize szGutter, BOOL FullWidth=FALSE);

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	RECT m_Categories[LFItemCategoryCount];
};

inline void CGridView::ResetItemCategories()
{
	ZeroMemory(m_Categories, sizeof(m_Categories));
}
