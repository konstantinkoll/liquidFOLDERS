
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// View IDs

#define LFViewLargeIcons                0
#define LFViewSmallIcons                1
#define LFViewList                      2
#define LFViewDetails                   3
#define LFViewTiles                     4
#define LFViewSearchResult              5
#define LFViewPreview                   6
#define LFViewCalendar                  7
#define LFViewGlobe                     8
#define LFViewTagcloud                  9
#define LFViewTimeline                  10

#define LFViewCount                     11


BOOL AttributeSortableInView(UINT Attr, UINT ViewMode);


// View parameters

struct LFViewParameters
{
	UINT Mode;
	INT ColumnOrder[LFAttributeCount];
	INT ColumnWidth[LFAttributeCount];

	UINT SortBy;
	BOOL Descending;
	BOOL AutoDirs;

	INT GlobeLatitude;
	INT GlobeLongitude;
	INT GlobeZoom;
	BOOL GlobeShowSpots;
	BOOL GlobeShowAirportNames;
	BOOL GlobeShowGPS;
	BOOL GlobeShowDescription;
	BOOL GlobeShowViewport;
	BOOL GlobeShowCrosshair;

	BOOL TagcloudCanonical;
	BOOL TagcloudHideRare;
	BOOL TagcloudUseSize;
	BOOL TagcloudUseColors;
	BOOL TagcloudUseOpacity;
};


// Item data

struct FVItemData
{
	RECT Rect;
	BOOL Selected;
};


// CFileView
//

#define WM_UPDATESELECTION    WM_USER+100
#define WM_SELECTALL          WM_USER+101
#define WM_SELECTNONE         WM_USER+102
#define WM_RENAMEITEM         WM_USER+103

class CFileView : public CWnd
{
public:
	CFileView(UINT DataSize=sizeof(FVItemData), BOOL EnableScrolling=TRUE, BOOL EnableHover=TRUE, BOOL EnableTooltip=TRUE, BOOL EnableShiftSelection=TRUE, BOOL EnableLabelEdit=TRUE);
	virtual ~CFileView();

	virtual CMenu* GetBackgroundContextMenu();
	virtual void EditLabel(INT idx);
	virtual BOOL IsEditing();

	BOOL Create(CWnd* pParentWnd, UINT nID, LFSearchResult* Result, INT FocusItem=0, UINT nClassStyle=CS_DBLCLKS);
	void UpdateViewOptions(INT Context=-1, BOOL Force=FALSE);
	void UpdateSearchResult(LFSearchResult* Result, INT FocusItem);
	INT GetFocusItem();
	INT GetSelectedItem();
	INT GetNextSelectedItem(INT idx);
	void SelectItem(INT idx, BOOL Select=TRUE, BOOL InternalCall=FALSE);
	void EnsureVisible(INT idx);

protected:
	INT m_Context;
	LFViewParameters m_ViewParameters;
	LFViewParameters* p_ViewParameters;
	LFSearchResult* p_Result;
	UINT m_DataSize;
	BYTE* m_ItemData;
	UINT m_ItemDataAllocated;
	LFDropTarget m_DropTarget;
	HTHEME hThemeList;
	LFTooltip m_TooltipCtrl;
	UINT m_HeaderHeight;
	INT m_FontHeight[2];
	BOOL m_EnableScrolling;
	BOOL m_EnableHover;
	BOOL m_EnableTooltip;
	BOOL m_EnableShiftSelection;
	BOOL m_EnableLabelEdit;
	BOOL m_HideFileExt;
	BOOL m_ShowFocusRect;
	INT m_FocusItem;
	INT m_HotItem;
	INT m_SelectionAnchor;
	INT m_EditLabel;
	INT m_ScrollWidth;
	INT m_ScrollHeight;
	INT m_HScrollPos;
	INT m_VScrollPos;
	INT m_HScrollMax;
	INT m_VScrollMax;
	INT m_RowHeight;
	BOOL m_Hover;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void SetViewOptions(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* Result);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT idx);
	virtual INT ItemAtPosition(CPoint point);
	virtual void InvalidateItem(INT idx);
	virtual CMenu* GetItemContextMenu(INT idx);

	void SetFocusItem(INT FocusItem, BOOL ShiftSelect);
	RECT GetItemRect(INT idx);
	void DrawItemBackground(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed);
	void PrepareFormatData(INT idx);
	void ResetScrollbars();
	void AdjustScrollbars();
	CString GetLabel(LFItemDescriptor* i);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg void OnDestroyEdit();
	afx_msg LRESULT OnItemsDropped(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CEdit* p_Edit;

	void AppendAttribute(LFItemDescriptor* i, UINT attr, CString& str);
	CString GetHint(LFItemDescriptor* i);
	void DestroyEdit(BOOL Accept=FALSE);
};
