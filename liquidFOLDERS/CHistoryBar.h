
#pragma once
#include "CFileView.h"
#include "LFCommDlg.h"


// Breadcrumbs
//

struct BreadcrumbItem
{
	BreadcrumbItem* pNext;
	LFFilter* pFilter;
	INT ContextID;
	FVPersistentData Data;
};

void AddBreadcrumbItem(BreadcrumbItem*& pBreadcrumbItem, LFFilter* pFilter, const FVPersistentData& Data);
void ConsumeBreadcrumbItem(BreadcrumbItem*& pBreadcrumbItem, LFFilter*& pFilter, FVPersistentData& Data);
void DeleteBreadcrumbItems(BreadcrumbItem*& pBreadcrumbItem);

inline void LeafBreadcrumbItem(BreadcrumbItem*& pAddItem, BreadcrumbItem*& pConsumeItem, LFFilter*& pFilter, FVPersistentData& Data)
{
	AddBreadcrumbItem(pAddItem, pFilter, Data);
	ConsumeBreadcrumbItem(pConsumeItem, pFilter, Data);
}


// CHistoryBar
//

#define WM_NAVIGATEBACK     WM_USER+201

class CHistoryBar : public CBackstageBar
{
public:
	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetHistory(const LFFilter* pFilter, BreadcrumbItem* pBreadcrumbItem);

protected:
	virtual void DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT State, BOOL Themed);
	virtual void OnClickButton(INT Index) const;

	void AddItem(const LFFilter* pFilter, CDC& dc);
};
