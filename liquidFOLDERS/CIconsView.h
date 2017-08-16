
// CIconsView.h: Schnittstelle der Klasse CIconsView
//

#pragma once
#include "CGridView.h"


// CIconsView
//

class CIconsView : public CGridView
{
public:
	CIconsView(UINT DataSize=sizeof(GridItemData));

protected:
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	void DrawWrapLabel(CDC& dc, const CRect& rectLabel, LFItemDescriptor* pItemDescriptor, UINT MaxLineCount=2) const;
};
