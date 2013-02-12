
// CTimelineView.h: Schnittstelle der Klasse CTimelineView
//

#pragma once
#include "CFileView.h"
#include "DynArray.h"


// CTimelineView
//

struct TimelineItemData
{
	FVItemData Hdr;
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
	INT m_PreviewCount;
	SIZE m_IconSize;

	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual void AdjustLayout();

	void DrawItem(CDC& dc, Graphics& g, LPRECT rectItem, INT idx, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
