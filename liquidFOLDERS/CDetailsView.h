
// CDetailsView.h: Schnittstelle der Klasse CDetailsView
//

#pragma once
#include "CGridView.h"


// CDetailsView
//

class CDetailsView : public CGridView
{
public:
	CDetailsView(UINT DataSize=sizeof(GridItemData));

protected:
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
};