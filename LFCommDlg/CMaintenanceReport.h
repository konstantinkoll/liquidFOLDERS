
// CMaintenanceReport.h: Schnittstelle der Klasse CMaintenanceReport
//

#pragma once
#include "CFrontstageWnd.h"
#include "LFCore.h"


// CMaintenanceReport
//

class CMaintenanceReport : public CFrontstageWnd
{
public:
	CMaintenanceReport();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetMaintenanceList(LFMaintenanceList* pMaintenanceList);

protected:
	void ResetScrollbars();
	void AdjustScrollbars();
	void AdjustLayout();
	void DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	LFMaintenanceList* p_MaintenanceList;
	CImageList* p_StoreIcons;
	HICON hIconReady;
	HICON hIconWarning;
	HICON hIconError;
	UINT m_ItemHeight;
	UINT m_IconSize;
	UINT m_BadgeSize;
	INT m_ScrollHeight;
	INT m_VScrollPos;
	INT m_VScrollMax;

private:
	WCHAR m_ErrorText[LFErrorCount][256];
	INT m_HotItem;
	BOOL m_Hover;
};
