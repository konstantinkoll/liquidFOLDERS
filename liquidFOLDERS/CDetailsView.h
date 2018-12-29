
// CDetailsView.h: Schnittstelle der Klasse CDetailsView
//

#pragma once
#include "CFileView.h"


// CDetailsView
//

class CDetailsView sealed : public CFileView
{
public:
	CDetailsView();

protected:
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual LFFont* GetLabelFont() const;
	virtual RECT GetLabelRect(INT Index) const;
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
};