
// LFStoreMaintenanceDlg.h: Schnittstelle der Klasse LFStoreMaintenanceDlg
//

#pragma once
#include "LFCore.h"
#include "CFrontstageItemView.h"


// CMaintenanceReport
//

class CMaintenanceReport sealed : public CFrontstageItemView
{
public:
	CMaintenanceReport();

	void SetMaintenanceList(LFMaintenanceList* pMaintenanceList);

protected:
	virtual void AdjustLayout();
	virtual void ShowTooltip(const CPoint& point);
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	CImageList* p_StoreIcons;
	LFMaintenanceList* p_MaintenanceList;

	UINT m_BadgeSize;
	HICON hIconReady;
	HICON hIconWarning;
	HICON hIconError;

private:
	COLORREF SetDarkTextColor(CDC& dc, INT Index, HICON hIcon, BOOL Themed) const;

	WCHAR m_ErrorText[LFErrorCount][256];
};

inline COLORREF CMaintenanceReport::SetDarkTextColor(CDC& dc, INT Index, HICON hIcon, BOOL Themed) const
{
	return (hIcon!=hIconReady) ? dc.SetTextColor(0x2020FF) : CFrontstageItemView::SetDarkTextColor(dc, Index, Themed);
}


// LFStoreMaintenanceDlg
//

class LFStoreMaintenanceDlg : public LFDialog
{
public:
	LFStoreMaintenanceDlg(LFMaintenanceList* pMaintenanceList, CWnd* pParentWnd=NULL);

protected:
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	CHeaderArea m_wndHeaderArea;
	CMaintenanceReport m_wndMaintenanceReport;
	LFMaintenanceList* m_pMaintenanceList;
};
