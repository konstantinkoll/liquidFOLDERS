
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"


// CTimelineView
//

#define PRV_NONE         0
#define PRV_COMMENTS     1
#define PRV_THUMBS       2
#define PRV_TITLE        4
#define PRV_ALBUM        8
#define PRV_SOURCE      16
#define PRV_FOLDER      32

struct TimelineItemData
{
	FVItemData Hdr;
	INT Arrow;
	INT ArrowOffs;
	INT PreviewRows;
	INT ListCount;
	WORD Year;
	LPCWSTR pArtist;
	LPCWSTR pTitle;
	LPCWSTR pAlbum;
	LPCWSTR pComments;
	BYTE Preview;
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
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
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
	INT m_ItemWidth;
	INT m_LabelWidth;
	INT m_PreviewColumns;
	INT m_IconSize;

private:
	static LPCWSTR GetAttribute(TimelineItemData* pData, LFItemDescriptor* pItemDesciptor, UINT Attr, UINT Mask);
	void DrawCategory(CDC& dc, Graphics& g, LPCRECT rectCategory, ItemCategory* ic, BOOL Themed);

	LFDynArray<ItemCategory, 8, 8> m_Categories;
	static CString m_FilesSingular;
	static CString m_FilesPlural;
	static CIcons m_SourceIcons;
};
