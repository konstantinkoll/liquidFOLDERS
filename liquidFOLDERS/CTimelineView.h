
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"


// CTimelineView
//

#define PRV_SOURCE             0x00000001
#define PRV_CONTENTS           0x00000100
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
	FVItemData Hdr;
	INT Arrow;
	INT ArrowOffs;
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

struct ItemCategory
{
	WCHAR Caption[5];
	RECT Rect;
};

class CTimelineView : public CFileView
{
public:
	CTimelineView();

protected:
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	BOOL m_TwoColumns;
	INT m_CaptionHeight;
	INT m_SourceHeight;
	INT m_ItemWidth;
	INT m_LabelWidth;
	INT m_BackgroundWidth;
	INT m_PreviewColumns;
	INT m_SmallIconSize;
	INT m_SourceIconSize;

private:
	static LPCWSTR GetAttribute(TimelineItemData* pData, UINT Mask, LFItemDescriptor* pItemDescriptor, UINT Attr);
	static void AggregateAttribute(UINT& PreviewMask, LPCWSTR& pStrAggregated, UINT Mask, LFItemDescriptor* pItemDescriptor, UINT Attr);
	static void AggregateIcon(UINT& PreviewMask, INT& AggregatedIconID, UINT Mask, INT IconID);
	static BOOL UsePreview(LFItemDescriptor* pItemDescriptor);
	void DrawCategory(CDC& dc, Graphics& g, LPCRECT rectCategory, ItemCategory* pItemCategory, BOOL Themed);

	LFDynArray<ItemCategory, 8, 8> m_Categories;
	static CIcons m_SourceIcons;
	static const ARGB m_BevelColors[8];
};
