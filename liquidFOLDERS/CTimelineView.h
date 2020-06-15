
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"


// CTimelineView
//

#define PRV_SOURCE             0x00000001
#define PRV_CONTENTLIST        0x00000100
#define PRV_THUMBNAILS         0x00000200
#define PRV_REPRESENTATIVE     0x00000400
#define PRV_CREATOR            0x00100000
#define PRV_TITLE              0x00200000
#define PRV_COLLECTION         0x00400000
#define PRV_COMMENTS           0x00800000
#define PRV_COLLECTIONICON     0x01000000

#define PRV_ATTRIBUTES         (PRV_CREATOR | PRV_TITLE | PRV_COLLECTION | PRV_COMMENTS)
#define PRV_PREVIEW            (PRV_THUMBNAILS | PRV_REPRESENTATIVE)
#define PRV_MIDDLESECTION      (PRV_CONTENTS | PRV_PREVIEW | PRV_ATTRIBUTES)

struct TimelineItemData
{
	ItemData Hdr;
	INT Arrow;
	INT ArrowOffs;
	INT ThumbnailCount;
	INT ThumbnailRows;
	INT ListCount;
	WORD Year;
	LPCWSTR pStrCreator;
	LPCWSTR pStrTitle;
	LPCWSTR pStrCollection;
	LPCWSTR pStrComments;
	INT CollectionIconID;
	UINT PreviewMask;
};

class CTimelineView sealed : public CFileView
{
public:
	CTimelineView();

protected:
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual INT HandleNavigationKeys(UINT nChar, BOOL Control) const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
	virtual void DrawStage(CDC& dc, Graphics& g, const CRect& rect, const CRect& rectUpdate, BOOL Themed);
	virtual RECT GetLabelRect() const;

	void DrawCategory(CDC& dc, Graphics& g, LPCRECT lpcRectCategory, ItemCategoryData* pItemCategoryData, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

	BOOL m_TwoColumns;
	INT m_CaptionHeight;
	INT m_ItemWidth;
	INT m_LabelWidth;
	INT m_PreviewColumns;
	INT m_SmallIconSize;

private:
	TimelineItemData* GetTimelineItemData(INT Index) const;
	static LPCWSTR GetAttribute(TimelineItemData* pData, UINT Mask, const LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr);
	static void AggregateAttribute(UINT& PreviewMask, LPCWSTR& pStrAggregated, UINT Mask, LFItemDescriptor* pItemDescriptor, ATTRIBUTE Attr);
	static void AggregateIcon(UINT& PreviewMask, INT& AggregatedIconID, UINT Mask, INT IconID);
	static BOOL UsePreview(LFItemDescriptor* pItemDescriptor);

	static const ARGB m_BevelColors[8];
};

inline TimelineItemData* CTimelineView::GetTimelineItemData(INT Index) const
{
	return (TimelineItemData*)GetItemData(Index);
}
