
// CGridView.h: Schnittstelle der Klasse CGridView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


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
	BOOL m_HasCategories;
	BOOL m_ForceNothing;
	UINT m_GridArrange;

	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed);

	void AddItemCategory(WCHAR* Caption, WCHAR* Name);
	void ResetItemCategories();
	void ArrangeHorizontal(GVArrange& gva, BOOL Justify=TRUE, BOOL ForceBreak=FALSE, BOOL MaxWidth=FALSE);
	void ArrangeVertical(GVArrange& gva);

	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	DynArray<ItemCategory> m_Categories;

	void HandleHorizontalKeys(UINT nChar, UINT nRepCnt, UINT nFlags);
	void HandleVerticalKeys(UINT nChar, UINT nRepCnt, UINT nFlags);
};
