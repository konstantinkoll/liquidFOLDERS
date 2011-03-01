
#pragma once
#include "liquidFOLDERS.h"
#include "CFileView.h"


// Breadcrumbs
//

struct BreadcrumbItem
{
	BreadcrumbItem* next;
	LFFilter* filter;
	FVPersistentData data;
};

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* f, FVPersistentData& data);
void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** f, FVPersistentData* data);
void DeleteBreadcrumbs(BreadcrumbItem** bi);


// CHistoryBar
//

class CHistoryBar : public CWnd
{
public:
	CHistoryBar();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CGlasWindow* pParentWnd, UINT nID);
	UINT GetPreferredHeight();

protected:
	BOOL m_Hover;
	LFTooltip m_TooltipCtrl;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//afx_msg void OnSetFocus(CWnd* pOldWnd);
	//afx_msg void OnKillFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	HTHEME hTheme;
};
