
#pragma once
#include "CFileView.h"
#include "LFCommDlg.h"


// Breadcrumbs
//

struct BreadcrumbItem
{
	BreadcrumbItem* pNext;
	LFFilter* pFilter;
	FVPersistentData Data;
};

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* pFilter, FVPersistentData& Data);
void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** ppFilter, FVPersistentData* Data);
void DeleteBreadcrumbs(BreadcrumbItem** bi);


// CHistoryBar
//

struct HistoryItem
{
	WCHAR Name[256];
	INT Width;
	INT Left;
	INT Right;
};

class CHistoryBar : public CWnd
{
public:
	CHistoryBar();

	BOOL Create(CGlassWindow* pParentWnd, UINT nID);
	UINT GetPreferredHeight();
	void SetHistory(LFFilter* ActiveFilter, BreadcrumbItem* Breadcrumbs);

protected:
	INT HitTest(CPoint point);
	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	BOOL m_IsEmpty;
	CString m_EmptyHint;
	INT m_Hover;
	INT m_Pressed;
	LFDynArray<HistoryItem> m_Breadcrumbs;

private:
	void AddFilter(LFFilter* Filter, CDC* pDC);
};
