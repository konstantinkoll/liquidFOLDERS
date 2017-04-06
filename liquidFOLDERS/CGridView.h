
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
	CGridView(UINT DataSize=sizeof(GridItemData), BOOL EnableLabelEdit=TRUE);

protected:
	virtual void DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed)=NULL;

	void ResetItemCategories();
	void Arrange(CSize szItem, INT Padding, CSize szGutter, BOOL FullWidth=FALSE);
	void DrawJumboIcon(CDC& dc, CRect& rect, LFItemDescriptor* pItemDescriptor);

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
