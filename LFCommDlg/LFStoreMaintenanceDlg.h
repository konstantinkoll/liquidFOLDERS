
// LFStoreMaintenanceDlg.h: Schnittstelle der Klasse LFStoreMaintenanceDlg
//

#pragma once
#include "LFCore.h"
#include "CFrontstageItemView.h"


// CMaintenanceReport
//

class CMaintenanceReport : public CFrontstageItemView
{
public:
	CMaintenanceReport();

	void SetMaintenanceList(LFMaintenanceList* pMaintenanceList);

protected:
	virtual void ShowTooltip(const CPoint& point);
	virtual void AdjustLayout();
	virtual void DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

	INT m_IconSize;
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
	return (hIcon!=hIconReady) ? dc.SetTextColor(0x0000FF) : CFrontstageItemView::SetDarkTextColor(dc, Index, Themed);
}


// LFStoreMaintenanceDlg
//

class LFStoreMaintenanceDlg : public LFDialog
{
public:
	LFStoreMaintenanceDlg(LFMaintenanceList* pMaintenanceList, CWnd* pParentWnd=NULL);
	~LFStoreMaintenanceDlg();

protected:
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	CHeaderArea m_wndHeaderArea;
	CMaintenanceReport m_wndMaintenanceReport;
	LFMaintenanceList* m_pMaintenanceList;
};
