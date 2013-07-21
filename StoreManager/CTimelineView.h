
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CTimelineView
//

#define PRV_NONE           0
#define PRV_COMMENTS       1
#define PRV_THUMBS         2
#define PRV_AUDIOTITLE     4
#define PRV_AUDIOALBUM     8

struct TimelineItemData
{
	FVItemData Hdr;
	INT Arrow;
	INT ArrowOffs;
	INT PreviewRows;
	WORD Year;
	WCHAR* pArtist;
	WCHAR* pTitle;
	WCHAR* pAlbum;
	WCHAR* pComments;
	BYTE Preview;
};

class CTimelineView : public CFileView
{
public:
	CTimelineView();

protected:
	BOOL m_TwoColumns;
	INT m_CaptionHeight;
	INT m_ItemWidth;
	INT m_LabelWidth;
	INT m_PreviewColumns;
	SIZE m_IconSize;

	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT idx);
	virtual void ScrollWindow(INT dx, INT dy);

	void DrawItem(CDC& dc, Graphics& g, LPRECT rectItem, INT idx, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_AttributeIcons;
	DynArray<ItemCategory> m_Categories;

	void DrawCategory(CDC& dc, Graphics& g, LPRECT rectCategory, ItemCategory* ic, COLORREF tlCol, BOOL Themed);
};
