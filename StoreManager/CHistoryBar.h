
#pragma once
#include "liquidFOLDERS.h"
#include "CFileView.h"
#include "DynArray.h"


// Breadcrumbs
//

struct BreadcrumbItem
{
	BreadcrumbItem* next;
	LFFilter* filter;
	FVPersistentData data;
};

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* filter, FVPersistentData& data);
void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** filter, FVPersistentData* data);
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

	BOOL Create(CGlasWindow* pParentWnd, UINT nID);
	UINT GetPreferredHeight();
	void SetHistory(LFFilter* ActiveFilter, BreadcrumbItem* Breadcrumbs);

protected:
	BOOL m_IsEmpty;
	CString m_EmptyHint;
	INT m_Hover;
	INT m_Pressed;
	DynArray<HistoryItem> m_Breadcrumbs;

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

private:
	void AddFilter(LFFilter* Filter, CDC* pDC);
};
