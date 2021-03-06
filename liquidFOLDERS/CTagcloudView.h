
// CTagcloudView.h: Schnittstelle der Klasse CTagcloudView
//

#pragma once
#include "CFileView.h"


// Item Data

struct TagcloudItemData
{
	ItemData Hdr;
	INT Cnt;
	COLORREF Color;
	UINT FontSize;
	UINT Alpha;
};


// CTagcloudView
//

class CTagcloudView sealed : public CFileView
{
public:
	CTagcloudView();

	virtual BOOL GetContextMenu(CMenu& Menu, INT Index);

protected:
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual COLORREF GetItemTextColor(INT Index, BOOL Themed) const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSortValue();
	afx_msg void OnSortCount();
	afx_msg void OnShowRare();
	afx_msg void OnUseSize();
	afx_msg void OnUseColor();
	afx_msg void OnUseOpacity();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	TagcloudItemData* GetTagcloudItemData(INT Index) const;
	LFFont* GetItemFont(INT Index);

	LFFont m_Fonts[20];
};

inline TagcloudItemData* CTagcloudView::GetTagcloudItemData(INT Index) const
{
	return (TagcloudItemData*)GetItemData(Index);
}
