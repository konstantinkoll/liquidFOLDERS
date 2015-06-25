
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

struct GVArrange
{
	INT cx;
	INT cy;
	INT mx;
	INT my;
	INT padding;
	INT gutterx;
	INT guttery;
};

#define GRIDARRANGE_HORIZONTAL     0
#define GRIDARRANGE_VERTICAL       1
#define GRIDARRANGE_CUSTOM         2

class CGridView : public CFileView
{
public:
	CGridView(UINT DataSize=sizeof(GridItemData), BOOL EnableLabelEdit=TRUE);

protected:
	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT Index, BOOL Themed)=0;

	void AddItemCategory(WCHAR* Caption, WCHAR* Name);
	void ResetItemCategories();
	void ArrangeHorizontal(GVArrange& gva, BOOL Justify=TRUE, BOOL ForceBreak=FALSE, BOOL MaxWidth=FALSE);
	void ArrangeVertical(GVArrange& gva);

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	BOOL m_HasCategories;
	UINT m_GridArrange;

private:
	void HandleHorizontalKeys(UINT nChar, UINT nRepCnt, UINT nFlags);
	void HandleVerticalKeys(UINT nChar, UINT nRepCnt, UINT nFlags);

	LFDynArray<ItemCategory> m_Categories;
};