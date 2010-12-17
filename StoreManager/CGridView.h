
// CGridView.h: Schnittstelle der Klasse CGridView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CGridView
//

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

struct ItemCategory
{
	WCHAR Caption[256];
	WCHAR Hint[256];
	RECT rect;
};

class CGridView : public CFileView
{
public:
	CGridView(UINT DataSize=sizeof(FVItemData));

protected:
	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed);

	void AddItemCategory(WCHAR* Caption, WCHAR* Name);
	void ResetItemCategories();
	void ArrangeHorizontal(GVArrange& gva, BOOL Justify=TRUE, BOOL ForceBreak=FALSE, BOOL MaxWidth=FALSE);
	void ArrangeVertical(GVArrange& gva);

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	DynArray<ItemCategory> m_Categories;

	void DrawCategory(CDC& dc, ItemCategory* ic, BOOL Themed);
};
