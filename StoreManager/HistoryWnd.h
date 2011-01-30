
#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"
#include "CPaneList.h"
#include "CFileView.h"


// Breadcrumbs
//

struct BreadcrumbItem {
	BreadcrumbItem* next;
	LFFilter* filter;
	FVPersistentData data;
};

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, FVPersistentData& data);
void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, FVPersistentData* data);
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
	CImageListTransparent m_Icons;
	INT m_CurrentItem;

	void AddFilterItem(LFFilter* f, BOOL append, BOOL focus=FALSE);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg void OnGotoHistory();
	afx_msg void OnNotifyGotoHistory(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
