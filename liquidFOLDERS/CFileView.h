
// CFileView.h: Schnittstelle der Klasse CFileView
//

#pragma once
#include "LFCommDlg.h"


// View parameters

#define VIEWSETTINGSVERSION     4

struct LFContextViewSettings
{
	UINT SortBy;
	BOOL Descending;

	UINT View;
	INT ColumnOrder[LFAttributeCount];
	INT ColumnWidth[LFAttributeCount];
};

struct LFGlobalViewSettings
{
	UINT LastViewSelected[LFAttributeCount];

	BOOL CalendarShowDays;

	INT GlobeLatitude;
	INT GlobeLongitude;
	INT GlobeZoom;
	BOOL GlobeShowSpots;
	BOOL GlobeShowAirportNames;
	BOOL GlobeShowGPS;
	BOOL GlobeShowDescription;

	BOOL TagcloudCanonical;
	BOOL TagcloudShowRare;
	BOOL TagcloudUseSize;
	BOOL TagcloudUseColors;
	BOOL TagcloudUseOpacity;
};

struct GlobeParameters
{
	GLfloat Latitude;
	GLfloat Longitude;
	INT Zoom;
};

struct FVPersistentData
{
	INT FocusItem;
	INT HScrollPos;
	INT VScrollPos;
	UINT Year;
	GlobeParameters Location;
	BOOL LocationValid;
};


// SendTo Item

struct SendToItemData
{
	HICON hIcon;
	INT cx;
	INT cy;
	BOOL IsStore;
	CHAR StoreID[LFKeySize];
	WCHAR Path[MAX_PATH];
};


// Item Data

struct FVItemData
{
	RECT Rect;
	INT RectInflate;
	BOOL Selected;
	BOOL Valid;
};


// Theming

struct CachedSelectionBitmap
{
	HBITMAP hBitmap;
	INT Width;
	INT Height;
};


// CFileView
//

#define WM_UPDATESELECTION    WM_USER+100
#define WM_SELECTALL          WM_USER+101
#define WM_SELECTNONE         WM_USER+102
#define WM_RENAMEITEM         WM_USER+103

#define BM_REFLECTION         0
#define BM_SELECTED           1

#define HORIZONTALSCROLLWIDTH     64

class CFileView : public CFrontstageWnd
{
public:
	CFileView(UINT DataSize=sizeof(FVItemData), BOOL EnableScrolling=TRUE, BOOL EnableHover=TRUE, BOOL EnableTooltip=TRUE, BOOL EnableShiftSelection=TRUE, BOOL EnableLabelEdit=TRUE, BOOL EnableTooltipOnVirtual=TRUE);
	virtual ~CFileView();

	virtual BOOL Create(CWnd* pParentWnd, UINT nID, const CRect& rect, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData=NULL, UINT nClassStyle=CS_DBLCLKS);
	virtual CMenu* GetViewContextMenu();
	virtual void GetPersistentData(FVPersistentData& Data) const;
	virtual void EditLabel(INT Index);

	void UpdateViewSettings(INT Context=-1, BOOL Force=FALSE);
	void UpdateSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, BOOL InternalCall=FALSE);
	INT GetFocusItem() const;
	INT GetSelectedItem() const;
	INT GetNextSelectedItem(INT Index) const;
	void SelectItem(INT Index, BOOL Select=TRUE, BOOL InternalCall=FALSE);
	void EnsureVisible(INT Index);
	BOOL MultiSelectAllowed() const;
	BOOL IsEditing() const;

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void SetViewSettings(BOOL Force);
	virtual void SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData);
	virtual void AdjustLayout();
	virtual RECT GetLabelRect(INT Index) const;
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual CMenu* GetItemContextMenu(INT Index);
	virtual void ScrollWindow(INT dx, INT dy, LPCRECT lpRect=NULL, LPCRECT lpClipRect=NULL);

	void ValidateAllItems();
	void SetFocusItem(INT FocusItem, BOOL ShiftSelect);
	RECT GetItemRect(INT Index) const;
	CMenu* GetSendToMenu();
	void DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	void DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached=TRUE);
	CString GetLabel(LFItemDescriptor* pItemDescriptor) const;
	BOOL BeginDragDrop();
	BOOL DrawNothing(CDC& dc, LPCRECT lpRectClient, BOOL Themed);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnMouseHWheel(UINT nFlags, SHORT zDelta, CPoint pt);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT Message);
	afx_msg void OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis);
	afx_msg void OnDrawItem(INT nID, LPDRAWITEMSTRUCT lpdis);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	afx_msg void OnSelectInvert();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	afx_msg void OnDestroyEdit();
	DECLARE_MESSAGE_MAP()

	INT m_Context;
	LFContextViewSettings m_ContextViewSettings;
	LFGlobalViewSettings m_GlobalViewSettings;
	LFContextViewSettings* p_ContextViewSettings;
	LFGlobalViewSettings* p_GlobalViewSettings;
	LFSearchResult* p_RawFiles;
	LFSearchResult* p_CookedFiles;
	UINT m_DataSize;
	BYTE* m_ItemData;
	UINT m_ItemDataAllocated;
	BOOL m_Nothing;
	UINT m_HeaderHeight;
	BOOL m_EnableScrolling;
	BOOL m_EnableHover;
	BOOL m_EnableTooltip;
	BOOL m_EnableShiftSelection;
	BOOL m_EnableLabelEdit;
	BOOL m_EnableTooltipOnVirtual;
	BOOL m_HideFileExt;
	BOOL m_ShowFocusRect;
	BOOL m_AllowMultiSelect;
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
	INT m_ColWidth;
	INT m_RowHeight;
	INT m_LargeFontHeight;
	INT m_DefaultFontHeight;
	INT m_SmallFontHeight;
	BOOL m_Hover;
	BOOL m_BeginDragDrop;
	CPoint m_DragPos;
	WCHAR m_TypingBuffer[256];
	DWORD m_TypingTicks;

private:
	void AppendSendToItem(CMenu* pMenu, UINT nIDCtl, LPCWSTR lpszNewItem, HICON hIcon, INT cx, INT cy);
	void ResetScrollbars();
	void AdjustScrollbars();
	void DestroyEdit(BOOL Accept=FALSE);

	CachedSelectionBitmap m_Bitmaps[2];
	CEdit* p_Edit;
	SendToItemData m_SendToItems[256];
};


inline BOOL CFileView::MultiSelectAllowed() const
{
	return m_AllowMultiSelect;
}

inline BOOL CFileView::IsEditing() const
{
	return (p_Edit!=NULL);
}
