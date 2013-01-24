
// CTagcloudView.h: Schnittstelle der Klasse CTagcloudView
//

#pragma once
#include "CGridView.h"


// Item data

struct TagcloudItemData
{
	GridItemData Hdr;
	INT Cnt;
	COLORREF Color;
	UINT FontSize;
	UINT Alpha;
};


// CTagcloudView
//

class CTagcloudView : public CGridView
{
public:
	CTagcloudView();

	virtual CMenu* GetBackgroundContextMenu();

protected:
	CFont m_Fonts[20];

	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data);
	virtual CBitmap* RenderFooter();
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed);

	CFont* GetFont(INT idx);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSortValue();
	afx_msg void OnSortCount();
	afx_msg void OnShowRare();
	afx_msg void OnShowLegend();
	afx_msg void OnUseSize();
	afx_msg void OnUseColors();
	afx_msg void OnUseOpacity();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
