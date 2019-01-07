
// CIconsView.h: Schnittstelle der Klasse CIconsView
//

#pragma once
#include "CFileView.h"


// CIconsView
//

class CIconsView sealed : public CFileView
{
public:
	CIconsView();

protected:
	virtual void SetViewSettings(BOOL UpdateSearchResultPending);
	virtual void SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);
	virtual RECT GetLabelRect(INT Index) const;

	afx_msg void OnShowCapacity();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	static void DrawCapacityBar(Graphics& g, const CRect& rect, const LFStoreDescriptor& StoreDescriptor);
	void DrawCapacity(CDC& dc, Graphics& g, CRect rectCapacity, const LFStoreDescriptor& StoreDescriptor, BOOL Themed) const;
	void DrawWrapLabel(CDC& dc, Graphics& g, const CRect& rectLabel, LFItemDescriptor* pItemDescriptor, BOOL Themed, UINT MaxLineCount=2) const;
};
