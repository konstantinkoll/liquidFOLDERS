
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

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetMaintenanceList(LFMaintenanceList* pMaintenanceList);

protected:
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual void ShowTooltip(const CPoint& point);

	void ResetScrollbars();
	void AdjustScrollbars();
	void AdjustLayout();
	void DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed) const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	LFMaintenanceList* p_MaintenanceList;
	UINT m_BadgeSize;
	HICON hIconReady;
	HICON hIconWarning;
	HICON hIconError;
	UINT m_ItemHeight;
	UINT m_IconSize;
	CImageList* p_StoreIcons;
	INT m_ScrollHeight;
	INT m_VScrollPos;
	INT m_VScrollMax;

private:
	WCHAR m_ErrorText[LFErrorCount][256];
};
