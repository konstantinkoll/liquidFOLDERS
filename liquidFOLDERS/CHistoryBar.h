
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

void AddBreadcrumbItem(BreadcrumbItem** pBreadcrumbItem, LFFilter* pFilter, FVPersistentData& Data);
void ConsumeBreadcrumbItem(BreadcrumbItem** pBreadcrumbItem, LFFilter** ppFilter, FVPersistentData* Data);
void DeleteBreadcrumbs(BreadcrumbItem** pBreadcrumbItem);


// CHistoryBar
//

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
