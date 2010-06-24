
#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// Breadcrumbs
//

struct BreadcrumbItem {
	BreadcrumbItem* next;
	LFFilter* filter;
	int focus;
};

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, int focus);
void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, int* focus);
void DeleteBreadcrumbs(BreadcrumbItem** bi);


// CHistoryWnd
//

class CHistoryWnd : public CDockablePane
{
public:
	CHistoryWnd();
	virtual ~CHistoryWnd();

	CPaneList m_wndList;

	void UpdateList(BreadcrumbItem* prev, LFFilter* current, BreadcrumbItem* next);

protected:
	CMFCToolBar m_wndToolBar;
	CImageListTransparent m_Icons;
	int m_CurrentItem;

	void AddFilterItem(LFFilter* f, BOOL focus=FALSE, BOOL append=TRUE);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg void OnGotoHistory();
	afx_msg void OnNotifyGotoHistory(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
