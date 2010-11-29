
// CGridView.h: Schnittstelle der Klasse CGridView
//

#pragma once
#include "CFileView.h"


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

class CGridView : public CFileView
{
public:
	CGridView(UINT DataSize=sizeof(FVItemData));

	virtual void EditLabel(INT idx);
	virtual BOOL IsEditing();

protected:
	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed);

	void ArrangeHorizontal(GVArrange& gva, BOOL Justify=TRUE, BOOL ForceBreak=FALSE, BOOL MaxWidth=FALSE);
	void ArrangeVertical(GVArrange& gva);

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
