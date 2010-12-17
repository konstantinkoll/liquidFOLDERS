
// CListView.h: Schnittstelle der Klasse CListView
//

#pragma once
#include "CGridView.h"
#include "LFCommDlg.h"


// CListView
//

class CListView : public CGridView
{
public:
	CListView(UINT DataSize=sizeof(FVItemData));

protected:
	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* Result);
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed);

	void DrawIcon(CDC& dc, CRect& rect, LFItemDescriptor* i, FVItemData* d);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

	CTooltipHeader m_wndHeader;

private:
	CImageList* m_Icons[2];
	SIZE m_IconSize[2];

	void AdjustHeader(BOOL bShow);
	void AttributeToString(LFItemDescriptor* i, UINT Attr, WCHAR* tmpStr, size_t cCount);
	void DrawTileRows(CDC& dc, CRect& rect, LFItemDescriptor* i, FVItemData* d, INT* Rows, BOOL Themed);
	void DrawProperty(CDC& dc, CRect& rect, LFItemDescriptor* i, FVItemData* d, UINT Attr, BOOL Themed);
	INT GetMaxLabelWidth(INT Max);
};
