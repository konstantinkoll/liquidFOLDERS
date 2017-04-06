
// CIconsView.h: Schnittstelle der Klasse CIconsView
//

#pragma once
#include "CGridView.h"
#include "LFCommDlg.h"


// CIconsView
//

class CIconsView : public CGridView
{
public:
	CIconsView(UINT DataSize=sizeof(GridItemData));

protected:
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual void DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed);
};
