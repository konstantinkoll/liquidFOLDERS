
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"


// CTimelineView
//

#define PRV_NONE           0
#define PRV_COMMENTS       1
#define PRV_THUMBS         2
#define PRV_TITLE          4
#define PRV_ALBUM          8
#define PRV_SOURCE        16

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
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index);
	virtual void ScrollWindow(INT dx, INT dy);

	void DrawItem(CDC& dc, Graphics& g, LPRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	BOOL m_TwoColumns;
	INT m_CaptionHeight;
	INT m_ItemWidth;
	INT m_LabelWidth;
	INT m_PreviewColumns;
	SIZE m_IconSize;

private:
	void DrawCategory(CDC& dc, Graphics& g, LPRECT rectCategory, ItemCategory* ic, BOOL Themed);

	CImageListTransparent m_AttributeIcons;
	LFDynArray<ItemCategory> m_Categories;
	CString m_FilesSingular;
	CString m_FilesPlural;
};
